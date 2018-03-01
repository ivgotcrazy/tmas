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
#include "obj_pool.hpp"

namespace BroadInter
{

class PktEntry;
class PktInfo;

/*******************************************************************************
 * 描  述: 数据分发器
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
class PktDispatcher : public boost::noncopyable
{
public:
	static PktDispatcher& GetInstance();

	~PktDispatcher();

	void Start();
	void Stop();

	void ProcessPacket(const PktEntry& pkt);

	void set_pkt_processor(const PktProcessorSP& processor) { pkt_processor_ = processor; }
	uint64 get_dropped_pkt_num() const { return dropped_pkt_num_; }
	void reset_dropped_pkt_num() { dropped_pkt_num_ = 0; }

private:
	PktDispatcher();

	void PktProcThreadFunc();

	typedef boost::shared_ptr<MultiThreadObjPool<PktInfo> > MultiThreadObjPoolSP;
	typedef boost::scoped_ptr<CircularFifo> CircularFifoCP;
	typedef std::list<PktInfoSP> PktQueue;
	typedef std::vector<ThreadSP> ThreadVec;
	
private:
	// 控制工作线程退出标志
	bool stop_flag_;

	// 报文捕获器从FIFO中获取可写入队列标志
	bool hold_recv_queue_;

	// 报文处理器
	PktProcessorSP pkt_processor_;

	// 报文捕获器写入数据队列指针
	PktQueue* recv_queue_;

	// 报文捕获器写入队列报文计数
	uint64 recv_pkt_num_;

	// 工作线程从FIFO中获取可读队列互斥量和条件变量
	boost::mutex proc_mutex_;
	boost::condition_variable proc_cond_;

	// 工作线程
	uint32 pkt_process_thread_count_;
	ThreadVec worker_threads_;

	// PktInfo对象池，由于PktInfo的释放依赖于此对象，如果对象池先析构了，
	// PktInfo的析构会导致coredump，因此需要使用shared_ptr存储。
	MultiThreadObjPoolSP pkt_info_pool_;

	// 报文读写队列FIFO缓冲
	CircularFifoCP pkt_queue_fifo_;

	uint64 dropped_pkt_num_;
};

}

#endif