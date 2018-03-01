/*#############################################################################
 * 文件名   : pkt_dispatcher.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : PktCapturer类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_DISPATCHER
#define BROADINTER_PKT_DISPATCHER

#include <vector>
#include <queue>
#include <list>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

#include "tmas_typedef.hpp"
#include "pkt_capturer.hpp"
#include "circular_fifo.hpp"
#include "object_pool.hpp"
#include "pkt_processor.hpp"

namespace BroadInter
{

class PktEntry;
class PktInfo;

/*******************************************************************************
 * 描  述: 数据分发器
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
template<class PktProcessorType>
class PktDispatcher : public boost::noncopyable
{
public:
	// 获取全局单例
	static boost::shared_ptr<PktDispatcher<PktProcessorType> > GetInstance();

	~PktDispatcher();

	// 初始化报文分发器
	bool Init();

	// 启动报文分发器
	void Start();

	// 停止报文分发器
	void Stop();

	// 接收报文捕获器捕获报文接口
	void ProcessPktEntry(const PktEntry& pkt);

	// 设置报文分发处理接口
	void SetPktProcessor(const boost::shared_ptr<PktProcessorType>& processor);

	uint64 get_dropped_pkt_num() const { return dropped_pkt_num_; }
	void reset_dropped_pkt_num() { dropped_pkt_num_ = 0; }

private:
	PktDispatcher();

	// 工作线程处理函数
	void PktProcThreadFunc();

	// 将报文队列做循环缓冲
	typedef std::list<PktInfoSP> PktQueue;
	
private:
	// 控制工作线程退出标志
	bool stop_flag_;

	// 报文捕获器从FIFO中获取可写入队列标志
	bool hold_recv_queue_;

	// 分发报文处理器
	boost::shared_ptr<PktProcessorType> pkt_processor_;

	// 报文捕获器写入数据队列指针
	PktQueue* recv_queue_;

	// 报文捕获器写入队列报文计数
	uint64 recv_pkt_num_;

	// 工作线程从FIFO中获取可读队列互斥量和条件变量
	boost::mutex proc_mutex_;
	boost::condition_variable proc_cond_;

	// 工作线程，即处理报文线程
	uint32 pkt_process_thread_count_;
	std::vector<ThreadSP> worker_threads_;

	// 报文对象池，供拷贝报文循环利用
	boost::shared_ptr<ObjectPool<PktInfo> > common_frame_pool_;
	boost::shared_ptr<ObjectPool<PktInfo> > jumbo_frame_pool_;

	// 包含报文循环缓冲队列的FIFO缓冲池
	boost::scoped_ptr<CircularFifo> pkt_queue_fifo_;

	// 循环缓冲池中缓冲队列容量大小
	uint32 capture_queue_size_;

	// 循环缓冲池中缓冲队列数量多少
	uint32 capture_queue_count_;

	// 处理繁忙导致丢弃的报文数
	uint64 dropped_pkt_num_;
};

}

#include "pkt_dispatcher-inl.hpp"

#endif