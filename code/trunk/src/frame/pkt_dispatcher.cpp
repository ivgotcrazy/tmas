/*#############################################################################
 * �ļ���   : pkt_dispatcher.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��20��
 * �ļ����� : PktCapturer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <algorithm>
#include <glog/logging.h>

#include "pkt_dispatcher.hpp"
#include "pkt_capturer.hpp"
#include "mem_buf_pool.hpp"	
#include "pkt_processor.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_cfg.hpp"
#include "pkt_resolver.hpp"
#include "pkt_receiver.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] index ����PktDistributor����
 *         [in] pkt_receivers ���ķַ����ն���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktDispatcher::PktDispatcher(uint8 index, PktReceiverVec& pkt_receivers) 
	: distributor_index_(index)
	, pkt_receivers_(pkt_receivers)
	, pkt_receiver_count_(pkt_receivers_.size())
	, policy_(DP_IP)
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktDispatcher::~PktDispatcher()
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktDispatcher::Init()
{
	// �������ļ���ȡ�ַ�����

	std::string policy;
	GET_TMAS_CONFIG_STR("global.netmap.packet-dispatch-policy", policy);

	if (policy == "rr" || policy == "round-robin")
	{
		LOG(INFO) << "Packet dispatch policy : rr(round-robin)";
		policy_ = DP_RR;
	}
	else if (policy == "ip")
	{
		LOG(INFO) << "Packet dispatch policy : ip";
		policy_ = DP_IP;
	}
	else if (policy == "ip-port")
	{
		LOG(INFO) << "Packet dispatch policy : ip-port";
		policy = DP_IP_PORT;
	}
	else
	{
		LOG(ERROR) << "Invalid packet-dispatch-policy : " << policy;
		return false;
	}

	// �������������У����
	if (pkt_receiver_count_ == 0)
	{
		LOG(ERROR) << "Empty packet receviers";
		return false;
	}

	ConstructDispatchFuncs();

	BatchDispatchInit();

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����ַ�������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::ConstructDispatchFuncs()
{
	dispatch_funcs_[DP_RR]		= &PktDispatcher::DispatchPktRoundRobin;
	dispatch_funcs_[DP_IP]		= &PktDispatcher::DispatchPktByIp;
	dispatch_funcs_[DP_IP_PORT] = &PktDispatcher::DispatchPktByIpAndPort;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����ַ���س�ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::BatchDispatchInit()
{
	batch_dispatch_funcs_[DP_RR]	  = &PktDispatcher::DispatchPktRoundRobinBatch;
	batch_dispatch_funcs_[DP_IP]	  = &PktDispatcher::DispatchPktByIpBatch;
	batch_dispatch_funcs_[DP_IP_PORT] = &PktDispatcher::DispatchPktByIpAndPortBatch;

	batch_pkt_cachers_.resize(pkt_receiver_count_);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����ַ�
 * ��  ��: [in] pkts ����
 *         [in] size ��������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktBatch(NetmapPktInfo** pkts, uint32 size)
{
	TMAS_ASSERT(policy_ < DP_BUTT);

	(this->*batch_dispatch_funcs_[policy_])(pkts, size);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ʵ�ֱ��ķַ�
 * ��  ��: [in] pkt_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPkt(const NetmapPktInfo& pkt_info)
{
	TMAS_ASSERT(policy_ < DP_BUTT);

	(this->*dispatch_funcs_[policy_])(pkt_info);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����IP��ַ�ַ�����
 * ��  ��: [in] pkt_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIp(const NetmapPktInfo& pkt_info)
{
	uint32 src_ip = PR::GetSourceIpN(pkt_info.data + 14);
	uint32 dst_ip = PR::GetDestIpN(pkt_info.data + 14);

	uint8 index = (src_ip + dst_ip) % pkt_receiver_count_;

	pkt_receivers_[index]->ReceivePkt(distributor_index_, pkt_info);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ѯ���еı��Ľ��������зַ�
 * ��  ��: [in] pkt_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktRoundRobin(const NetmapPktInfo& pkt_info)
{
	static uint8 current_receiver = 0;

	pkt_receivers_[current_receiver]->ReceivePkt(distributor_index_, pkt_info);

	current_receiver = (current_receiver + 1) % pkt_receiver_count_;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����IP��Port�ַ�����
 * ��  ��: [in] pkt_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIpAndPort(const NetmapPktInfo& pkt_info)
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ѯ���еı��Ľ��������������ַ�
 * ��  ��: [in] pkts ����
 *         [in] size ��������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktRoundRobinBatch(NetmapPktInfo** pkts, uint32 size)
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ����IP��ַ�������ַ�
 * ��  ��: [in] pkts ����
 *         [in] size ��������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIpBatch(NetmapPktInfo** pkts, uint32 size)
{
	// �Ȱ��շַ����Խ���Ԥ�ַ��������Ĵ����һ��
	for (uint32 i = 0; i < size; i++)
	{
		uint32 src_ip = PR::GetSourceIpN(pkts[i]->data + 14);
		uint32 dst_ip = PR::GetDestIpN(pkts[i]->data + 14);

		uint8 receiver_index = (src_ip + dst_ip) % pkt_receiver_count_;

		batch_pkt_cachers_[receiver_index].push_back(pkts[i]);
	}

	// ������õı���һ���Էַ���receiver
	for (uint8 i = 0; i < pkt_receiver_count_; i++)
	{
		// �п���ĳһ��receiver��û�зַ�������
		if (batch_pkt_cachers_[i].empty()) continue;

		pkt_receivers_[i]->ReceivePktBatch(distributor_index_, batch_pkt_cachers_[i]);

		batch_pkt_cachers_[i].clear();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����IP��Port�������ַ�
 * ��  ��: [in] pkts ����
 *         [in] size ��������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIpAndPortBatch(NetmapPktInfo** pkts, uint32 size)
{

}

}