/*#############################################################################
 * �ļ���   : pkt_receiver.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��19��
 * �ļ����� : PktReceiver������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_RECEIVER
#define BROADINTER_PKT_RECEIVER

#include <atomic>
#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "frame_typedef.hpp"
#include "circular_fifo.hpp"
#include "timer.hpp"
#include "tmas_assert.hpp"
#include "pkt_capturer.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: �����ĵĲ�����ٷַ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��04��19��
 ******************************************************************************/
class PktReceiver : public boost::noncopyable
{
public:
	PktReceiver(uint8 receiver_index, uint8 distributor_count);

	bool Init();

	void Start();
	void Stop();

	inline void ReceivePkt(uint8 index, const NetmapPktInfo& pkt_info);
	
	inline void ReceivePktBatch(uint8 index, const std::vector<NetmapPktInfo*>& pkts);

private:
	// ���챨�Ĵ�����
	bool ConstructPktProcessor();

	// ���컷�λ�����
	bool ConstructPktFifo();

	// ���Ĵ����߳�
	void ThreadFunc();

	void OneByOneThreadProc();

	void BatchThreadProc();

	void DebugProc();

private:
	uint8 receiver_index_;

	// ���ݷַ����������������Ļ�����
	uint8 distributor_count_;
	
	// ���Ĵ�����
	EthMonitorTypeSP pkt_processor_;

	// ���Ļ�������ÿ����������һ���������
	typedef boost::shared_ptr<CircularFifo<NetmapPktInfo> > PktFifoSP;
	std::vector<PktFifoSP> pkt_fifos_;

	// ���Ĵ����߳�
	boost::shared_ptr<boost::thread> proc_thread_;

	// ���̰߳󶨵�CPU����
	bool bind_cpu_core_;
	uint8 cpu_core_index_;

	// �̺߳Ͷ�ʱ���˳���־
	bool stop_flag_;

	// �Ƿ�������������ģʽ
	bool batch_proc_;

	// debug���
	boost::scoped_ptr<FreeTimer> debug_timer_;
	uint64 dropped_pkt_num_;
	uint64 received_pkt_num_;
};

/*-----------------------------------------------------------------------------
 * ��  ��: ���ձ��ģ��������Ŀ��������λ�����
           �ڶ�����ץ�������£����ж���̷߳��ʴ˺������������������Ѿ�Ϊÿ��
		   ������������һ�����λ���������ˣ�ÿ���̷߳��ʵĻ��λ������Ƕ����ģ�
		   ���������ͬʱ�������̶߳Ի���������ֻ������ˣ��Ի����������ķ���
		   Ҳ����Ҫ������
 * ��  ��: [in] index �ַ�������(��Ӧһ��ץ������)
 *         [in] pkt_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
inline void PktReceiver::ReceivePkt(uint8 index, const NetmapPktInfo& pkt_info)
{
	TMAS_ASSERT(index < pkt_fifos_.size());

	NetmapPktInfo* fifo_pkt_info = pkt_fifos_[index]->GetWritableItem();
	if (!fifo_pkt_info)
	{
		dropped_pkt_num_++;
		pkt_info.pkt_capturer->PktProcessed(
			pkt_info.ring_index, pkt_info.slot_index);
		return;
	}

	std::memcpy(fifo_pkt_info, &pkt_info, sizeof(NetmapPktInfo));

	pkt_fifos_[index]->FinishedWrite();
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������ձ���
 * ��  ��: [in] index �ַ�������(��Ӧһ��ץ������)
 *         [in] pkts ����ָ������
 *         [in] size ���ĸ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
inline void PktReceiver::ReceivePktBatch(uint8 index, const std::vector<NetmapPktInfo*>& pkts)
{
	TMAS_ASSERT(index < pkt_fifos_.size());

	uint32 pkt_num = pkts.size();

	std::vector<NetmapPktInfo*> pkt_infos;
	if (!pkt_fifos_[index]->GetMultiWritableItems(pkt_num, pkt_infos))
	{
		dropped_pkt_num_++;

		for (NetmapPktInfo* pkt_info : pkts)
		{
			pkt_info->pkt_capturer->PktProcessed(
				pkt_info->ring_index, pkt_info->slot_index);
		}

		return;
	}

	for (uint32 i = 0; i < pkt_num; i++)
	{
		std::memcpy(pkt_infos[i], pkts[i], sizeof(NetmapPktInfo));
	}

	pkt_fifos_[index]->FinishedWriteMulti(pkt_num);
}

}

#endif