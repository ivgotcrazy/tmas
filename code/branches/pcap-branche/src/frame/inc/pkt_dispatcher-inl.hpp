/*#############################################################################
 * �ļ���   : pkt_dispatcher-inl.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : PktCapturer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ��ȡ����
 * ��  ��: 
 * ����ֵ: ����
 * ��  ��:
 *   ʱ�� 2014��03��15��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��15��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��15��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
PktDispatcher<PktProcessorType>::~PktDispatcher()
{
	if (!stop_flag_) Stop();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ñ��Ĵ�����
 * ��  ��: [in] processor ���Ĵ�����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��26��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::SetPktProcessor(const boost::shared_ptr<PktProcessorType>& processor)
{
	pkt_processor_ = processor;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ�����ķַ���
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ�� 
 * ��  ��:
 *   ʱ�� 2014��03��27��
 *   ���� teck_zhou
 *   ���� ����
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

	// ��ͨ������Ϣ�����
	common_frame_pool_.reset(new ObjectPool<PktInfo>(
		[](void)->PktInfo*{ return new PktInfo(COMMON_FRAME_MAX_SIZE); },
		[](PktInfo* p)->void{ delete p; },
		[](PktInfo* p)->void{ return; }, 128 * 128));

	// Ӧ��jumbo frame�ı�����Ϣ�����
	jumbo_frame_pool_.reset(new ObjectPool<PktInfo>(
		[](void)->PktInfo*{ return new PktInfo(JUMBO_FRAME_MAX_SIZE); },
		[](PktInfo* p)->void{ delete p; },
		[](PktInfo* p)->void{ return; }));

	// �������Ĳ���ѭ���������
	pkt_queue_fifo_.reset(new CircularFifo(
		[](void)->void*{ return new PktQueue; }, 
		[](void* p)->void{ delete (PktQueue*)p; },
		capture_queue_count_));

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������ķַ�
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ֹͣ���ķַ�
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��15��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::Stop()
{
	stop_flag_ = true;

	// �ȴ������߳��˳�
	for (ThreadSP& t : worker_threads_)
	{
		t->interrupt();
		t->join();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĳ�������
 * ��  ��: [in] pkt ����ı���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::ProcessPktEntry(const PktEntry& pkt)
{
	if (!hold_recv_queue_)
	{
		// ��FIFO�л�ȡ��д����
		if (pkt_queue_fifo_->GetWritableItem((void**)(&recv_queue_)))
		{
			hold_recv_queue_ = true;
		}
		else // �޿�д����
		{
			LOG(ERROR) << "!!! Drop packet " << ++dropped_pkt_num_;
			return;
		}
	}

	// �Ӷ�����л�ȡ���Ķ���
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

	// ��PCAP������������
	std::memcpy(pkt_info->pkt.buf, pkt.buf, pkt.len);
	pkt_info->pkt.len = pkt.len;

	// �����Ļ�����FIFO������
	recv_queue_->push_back(pkt_info);

	if (++recv_pkt_num_ >= capture_queue_size_)
	{
		pkt_queue_fifo_->FinishedWrite(); // ֪ͨFIFO�����Ѿ�д��

		proc_cond_.notify_all(); // ֪ͨ�����߳��пɶ�����

		hold_recv_queue_ = false; // ���»�ȡ��д����

		recv_pkt_num_ = 0; // ���¼���
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ĳ���
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class PktProcessorType>
void PktDispatcher<PktProcessorType>::PktProcThreadFunc()
{
	PktQueue* read_queue = nullptr;
	size_t queue_index = 0;

	while (!stop_flag_)
	{
		// ��FIFO��ȡ�ɶ����Ķ���
		{
			boost::mutex::scoped_lock lock(proc_mutex_);
			while (!pkt_queue_fifo_->GetReadableItem((void**)(&read_queue), queue_index))
			{
				proc_cond_.wait(lock);
			}
		}

		TMAS_ASSERT(read_queue);

		// ��������б���
		while (!read_queue->empty())
		{
			PktInfoSP& pkt_info = read_queue->front();
			
			pkt_processor_->ProcessPkt(pkt_info);
			
			read_queue->pop_front();
		}
		
		// ֪ͨFIFO�����Ѿ�����
		pkt_queue_fifo_->FinishedRead(queue_index);
	}
}

}

#endif