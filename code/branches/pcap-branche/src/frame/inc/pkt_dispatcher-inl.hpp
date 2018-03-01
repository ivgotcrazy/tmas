/*#############################################################################
 * 文件名   : pkt_dispatcher-inl.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : PktCapturer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef	BROADINTER_PKT_DISPATCHER_INL
#define BROADINTER_PKT_DISPATCHER_INL

#include <glog/logging.h>

#include "pkt_dispatcher.hpp"
#include "pkt_capturer.hpp"
#include "mem_buf_pool.hpp"	
#include "pkt_processor.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_cfg.hpp"
#include "eth_monitor.hpp"
#include "ipv4_monitor.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "http_monitor.hpp"

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
template<class PktProcessorType>
boost::shared_ptr<PktDispatcher<PktProcessorType> > 
PktDispatcher<PktProcessorType>::GetInstance()
{
	static boost::shared_ptr<PktDispatcher<PktProcessorType> > dispatcher(
		new PktDispatcher<PktProcessorType>());

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
template<class PktProcessorType>
PktDispatcher<PktProcessorType>::PktDispatcher() 
	: stop_flag_(false)
	, hold_recv_queue_(false)
	, recv_pkt_num_(0)
	, pkt_process_thread_count_(0)
	, dropped_pkt_num_(0)
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
template<class PktProcessorType>
PktDispatcher<PktProcessorType>::~PktDispatcher()
{
	if (!stop_flag_) Stop();
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置报文处理器
 * 参  数: [in] processor 报文处理器
 * 返回值: 
 * 修  改:
 *   时间 2014年03月26日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::SetPktProcessor(const boost::shared_ptr<PktProcessorType>& processor)
{
	pkt_processor_ = processor;
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化报文分发器
 * 参  数: 
 * 返回值: 成功/失败 
 * 修  改:
 *   时间 2014年03月27日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
bool PktDispatcher<PktProcessorType>::Init()
{
	GET_TMAS_CONFIG_INT("global.common.capture-queue-size", capture_queue_size_);
	if (capture_queue_size_ == 0)
	{
		LOG(ERROR) << "Invalid circular buffer queue size " << capture_queue_size_;
		return false;
	}

	GET_TMAS_CONFIG_INT("global.common.capture-queue-count", capture_queue_count_);
	if (capture_queue_count_ == 0)
	{
		LOG(ERROR) << "Invalid circular buffer queue count " << capture_queue_count_;
		return false;
	}

	GET_TMAS_CONFIG_INT("global.common.packet-process-thread-count", pkt_process_thread_count_);
	if (pkt_process_thread_count_ == 0 || pkt_process_thread_count_ > 128)
	{
		LOG(ERROR) << "Invalid packet process thread count " << pkt_process_thread_count_;
		return false;
	}

	// 普通报文消息对象池
	common_frame_pool_.reset(new ObjectPool<PktInfo>(
		[](void)->PktInfo*{ return new PktInfo(COMMON_FRAME_MAX_SIZE); },
		[](PktInfo* p)->void{ delete p; },
		[](PktInfo* p)->void{ return; }, 128 * 128));

	// 应对jumbo frame的报文消息对象池
	jumbo_frame_pool_.reset(new ObjectPool<PktInfo>(
		[](void)->PktInfo*{ return new PktInfo(JUMBO_FRAME_MAX_SIZE); },
		[](PktInfo* p)->void{ delete p; },
		[](PktInfo* p)->void{ return; }));

	// 创建报文捕获循环缓冲队列
	pkt_queue_fifo_.reset(new CircularFifo(
		[](void)->void*{ return new PktQueue; }, 
		[](void* p)->void{ delete (PktQueue*)p; },
		capture_queue_count_));

	return true;
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
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::Start()
{
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
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::Stop()
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
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::ProcessPktEntry(const PktEntry& pkt)
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
			LOG(ERROR) << "!!! Drop packet " << ++dropped_pkt_num_;
			return;
		}
	}

	// 从对象池中获取报文对象
	PktInfoSP pkt_info;
	if (pkt.len <= COMMON_FRAME_MAX_SIZE)
	{
		pkt_info = common_frame_pool_->AllocObject();
	}
	else if (pkt.len > COMMON_FRAME_MAX_SIZE && pkt.len < JUMBO_FRAME_MAX_SIZE)
	{
		pkt_info = jumbo_frame_pool_->AllocObject();

		DLOG(WARNING) << "Jumbo frame | size: " << pkt.len;
	}
	else
	{
		LOG(ERROR) << "Too big frame size " << pkt.len;
		return;
	}

	// 从PCAP拷贝报文数据
	std::memcpy(pkt_info->pkt.buf, pkt.buf, pkt.len);
	pkt_info->pkt.len = pkt.len;

	// 将报文缓冲至FIFO队列中
	recv_queue_->push_back(pkt_info);

	if (++recv_pkt_num_ >= capture_queue_size_)
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
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::PktProcThreadFunc()
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
			
			pkt_processor_->ProcessPkt(pkt_info);
			
			read_queue->pop_front();
		}
		
		// 通知FIFO队列已经读完
		pkt_queue_fifo_->FinishedRead(queue_index);
	}
}

}

#endif