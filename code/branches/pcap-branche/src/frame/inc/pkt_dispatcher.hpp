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
#include "object_pool.hpp"
#include "pkt_processor.hpp"

namespace BroadInter
{

class PktEntry;
class PktInfo;

/*******************************************************************************
 * ��  ��: ���ݷַ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
template<class PktProcessorType>
class PktDispatcher : public boost::noncopyable
{
public:
	// ��ȡȫ�ֵ���
	static boost::shared_ptr<PktDispatcher<PktProcessorType> > GetInstance();

	~PktDispatcher();

	// ��ʼ�����ķַ���
	bool Init();

	// �������ķַ���
	void Start();

	// ֹͣ���ķַ���
	void Stop();

	// ���ձ��Ĳ����������Ľӿ�
	void ProcessPktEntry(const PktEntry& pkt);

	// ���ñ��ķַ�����ӿ�
	void SetPktProcessor(const boost::shared_ptr<PktProcessorType>& processor);

	uint64 get_dropped_pkt_num() const { return dropped_pkt_num_; }
	void reset_dropped_pkt_num() { dropped_pkt_num_ = 0; }

private:
	PktDispatcher();

	// �����̴߳�����
	void PktProcThreadFunc();

	// �����Ķ�����ѭ������
	typedef std::list<PktInfoSP> PktQueue;
	
private:
	// ���ƹ����߳��˳���־
	bool stop_flag_;

	// ���Ĳ�������FIFO�л�ȡ��д����б�־
	bool hold_recv_queue_;

	// �ַ����Ĵ�����
	boost::shared_ptr<PktProcessorType> pkt_processor_;

	// ���Ĳ�����д�����ݶ���ָ��
	PktQueue* recv_queue_;

	// ���Ĳ�����д����б��ļ���
	uint64 recv_pkt_num_;

	// �����̴߳�FIFO�л�ȡ�ɶ����л���������������
	boost::mutex proc_mutex_;
	boost::condition_variable proc_cond_;

	// �����̣߳����������߳�
	uint32 pkt_process_thread_count_;
	std::vector<ThreadSP> worker_threads_;

	// ���Ķ���أ�����������ѭ������
	boost::shared_ptr<ObjectPool<PktInfo> > common_frame_pool_;
	boost::shared_ptr<ObjectPool<PktInfo> > jumbo_frame_pool_;

	// ��������ѭ��������е�FIFO�����
	boost::scoped_ptr<CircularFifo> pkt_queue_fifo_;

	// ѭ��������л������������С
	uint32 capture_queue_size_;

	// ѭ��������л��������������
	uint32 capture_queue_count_;

	// ����æ���¶����ı�����
	uint64 dropped_pkt_num_;
};

}

#include "pkt_dispatcher-inl.hpp"

#endif