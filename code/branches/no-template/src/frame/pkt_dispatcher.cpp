/*#############################################################################
 * �ļ���   : pkt_dispatcher.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : PktCapturer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ��ȡ����
 * ��  ��: 
 * ����ֵ: ����
 * ��  ��:
 *   ʱ�� 2014��03��15��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktDispatcher& PktDispatcher::GetInstance()
{
	static PktDispatcher dispatcher;
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
PktDispatcher::PktDispatcher() : stop_flag_(false), hold_recv_queue_(false),
	recv_pkt_num_(0), pkt_process_thread_count_(0), dropped_pkt_num_(0)
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
PktDispatcher::~PktDispatcher()
{
	if (!stop_flag_) Stop();
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
void PktDispatcher::Start()
{
	// ���������
	pkt_info_pool_.reset(new MultiThreadObjPool<PktInfo>());

	// �������ļ���ȡ�����̸߳���
	GET_TMAS_CONFIG_INT("global.common.packet-process-thread-count", pkt_process_thread_count_);
	if (pkt_process_thread_count_ == 0)
	{
		LOG(WARNING) << "Invalid packet process thread count";
		pkt_process_thread_count_ = 1;
	}

	// �������Ķ���FIFO��������еĸ�������Ϊ�������̸߳��� * 2 + 1
	// ���������ǣ�д���е��ٶȿ��ܻ�ȶ����п죬���ٱ�֤���й����߳�
	// �����ڶ�����ʱ��FIFO�л����㹻�Ŀ�д���С�
	pkt_queue_fifo_.reset(new CircularFifo(
						  [](void)->void*{ return new PktQueue; }, 
						  [](void* p)->void{ delete (PktQueue*)p; },
						  pkt_process_thread_count_ * 2 + 1));

	// ���������߳�
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
void PktDispatcher::Stop()
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
void PktDispatcher::ProcessPacket(const PktEntry& pkt)
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
			LOG(WARNING) << "Drop packet!!!";
			dropped_pkt_num_++;
			return;
		}
	}

	// �Ӷ�����л�ȡ���Ķ���
	PktInfoSP pkt_info(pkt_info_pool_->AllocObj(), 
		boost::bind(&MultiThreadObjPool<PktInfo>::FreeObj, pkt_info_pool_, _1));

	// ��PCAP������������
	std::memcpy(pkt_info->pkt.buf, pkt.buf, pkt.len);
	pkt_info->pkt.len = pkt.len;

	// �����Ļ�����FIFO������
	recv_queue_->push_back(pkt_info);

	if (++recv_pkt_num_ > PKT_QUEUE_MAX_SIZE)
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
void PktDispatcher::PktProcThreadFunc()
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
			
			pkt_processor_->Process(MSG_PKT, VOID_SHARED(pkt_info));

			read_queue->pop_front();
		}
		
		// ֪ͨFIFO�����Ѿ�����
		pkt_queue_fifo_->FinishedRead(queue_index);
	}
}

}
