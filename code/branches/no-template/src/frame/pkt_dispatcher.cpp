/*#############################################################################
 * 文件名   : pkt_dispatcher.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : PktCapturer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "pkt_dispatcher.hpp"
#include "pkt_capturer.hpp"
#include "mem_buf_pool.hpp"	
#include "pkt_processor.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 获取单例
 * 参  数: 
 * 返回值: 单例
 * 修  改:
 *   时间 2014年03月15日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktDispatcher& PktDispatcher::GetInstance()
{
	static PktDispatcher dispatcher;
	return dispatcher;
}

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月15日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktDispatcher::PktDispatcher() : stop_flag_(false), hold_recv_queue_(false),
	recv_pkt_num_(0), pkt_process_thread_count_(0), dropped_pkt_num_(0)
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月15日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktDispatcher::~PktDispatcher()
{
	if (!stop_flag_) Stop();
}

/*-----------------------------------------------------------------------------
 * 描  述: 启动报文分发
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::Start()
{
	// 创建对象池
	pkt_info_pool_.reset(new MultiThreadObjPool<PktInfo>());

	// 从配置文件读取工作线程个数
	GET_TMAS_CONFIG_INT("global.common.packet-process-thread-count", pkt_process_thread_count_);
	if (pkt_process_thread_count_ == 0)
	{
		LOG(WARNING) << "Invalid packet process thread count";
		pkt_process_thread_count_ = 1;
	}

	// 创建报文队列FIFO，缓冲队列的个数设置为：工作线程个数 * 2 + 1
	// 基本考虑是，写队列的速度可能会比读队列快，至少保证所有工作线程
	// 都正在读队列时，FIFO中还有足够的可写队列。
	pkt_queue_fifo_.reset(new CircularFifo(
						  [](void)->void*{ return new PktQueue; }, 
						  [](void* p)->void{ delete (PktQueue*)p; },
						  pkt_process_thread_count_ * 2 + 1));

	// 创建工作线程
	for (uint8 i = 0; i < pkt_process_thread_count_; i++)
	{
		worker_threads_.push_back(
			ThreadSP(new boost::thread(
				boost::bind(&PktDispatcher::PktProcThreadFunc, this))));
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 停止报文分发
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月15日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::Stop()
{
	stop_flag_ = true;

	// 等待工作线程退出
	for (ThreadSP& t : worker_threads_)
	{
		t->interrupt();
		t->join();
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文捕获处理函数
 * 参  数: [in] pkt 捕获的报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::ProcessPacket(const PktEntry& pkt)
{
	if (!hold_recv_queue_)
	{
		// 从FIFO中获取可写队列
		if (pkt_queue_fifo_->GetWritableItem((void**)(&recv_queue_)))
		{
			hold_recv_queue_ = true;
		}
		else // 无可写队列
		{
			LOG(WARNING) << "Drop packet!!!";
			dropped_pkt_num_++;
			return;
		}
	}

	// 从对象池中获取报文对象
	PktInfoSP pkt_info(pkt_info_pool_->AllocObj(), 
		boost::bind(&MultiThreadObjPool<PktInfo>::FreeObj, pkt_info_pool_, _1));

	// 从PCAP拷贝报文数据
	std::memcpy(pkt_info->pkt.buf, pkt.buf, pkt.len);
	pkt_info->pkt.len = pkt.len;

	// 将报文缓冲至FIFO队列中
	recv_queue_->push_back(pkt_info);

	if (++recv_pkt_num_ > PKT_QUEUE_MAX_SIZE)
	{
		pkt_queue_fifo_->FinishedWrite(); // 通知FIFO队列已经写满

		proc_cond_.notify_all(); // 通知工作线程有可读队列

		hold_recv_queue_ = false; // 重新获取可写队列

		recv_pkt_num_ = 0; // 重新计数
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 启动报文捕获
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::PktProcThreadFunc()
{
	PktQueue* read_queue = nullptr;
	size_t queue_index = 0;

	while (!stop_flag_)
	{
		// 从FIFO获取可读报文队列
		{
			boost::mutex::scoped_lock lock(proc_mutex_);
			while (!pkt_queue_fifo_->GetReadableItem((void**)(&read_queue), queue_index))
			{
				proc_cond_.wait(lock);
			}
		}

		TMAS_ASSERT(read_queue);

		// 处理队列中报文
		while (!read_queue->empty())
		{
			PktInfoSP& pkt_info = read_queue->front();
			
			pkt_processor_->Process(MSG_PKT, VOID_SHARED(pkt_info));

			read_queue->pop_front();
		}
		
		// 通知FIFO队列已经读完
		pkt_queue_fifo_->FinishedRead(queue_index);
	}
}

}
