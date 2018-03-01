/*#############################################################################
 * �ļ���   : pkt_dispatcher.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : PktCapturer������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ���ݷַ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
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
	// ���ƹ����߳��˳���־
	bool stop_flag_;

	// ���Ĳ�������FIFO�л�ȡ��д����б�־
	bool hold_recv_queue_;

	// ���Ĵ�����
	PktProcessorSP pkt_processor_;

	// ���Ĳ�����д�����ݶ���ָ��
	PktQueue* recv_queue_;

	// ���Ĳ�����д����б��ļ���
	uint64 recv_pkt_num_;

	// �����̴߳�FIFO�л�ȡ�ɶ����л���������������
	boost::mutex proc_mutex_;
	boost::condition_variable proc_cond_;

	// �����߳�
	uint32 pkt_process_thread_count_;
	ThreadVec worker_threads_;

	// PktInfo����أ�����PktInfo���ͷ������ڴ˶������������������ˣ�
	// PktInfo�������ᵼ��coredump�������Ҫʹ��shared_ptr�洢��
	MultiThreadObjPoolSP pkt_info_pool_;

	// ���Ķ�д����FIFO����
	CircularFifoCP pkt_queue_fifo_;

	uint64 dropped_pkt_num_;
};

}

#endif