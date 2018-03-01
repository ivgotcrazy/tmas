/*#############################################################################
 * �ļ���   : eth_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��21��
 * �ļ����� : EthMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_ETH_MONITOR_INL
#define BROADINTER_ETH_MONITOR_INL

#include <glog/logging.h>

#include "eth_monitor.hpp"
#include "pkt_processor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "pkt_parser.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��01��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool EthMonitor<Next, Succ>::Init()
{
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ:
 * ��  ��: 
 *   ʱ�� 2014��01��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo EthMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	// ����ETHͷ��Ϣ
	if (!ParsePktEthInfo(pkt_msg))
	{
		LOG(ERROR) << "Fail to resolve L2 info";
		return PI_NOT_PROCESSED;
	}

	PrintEthInfo(pkt_msg);

	this->PassPktToSuccProcessor(pkt_msg); // ���¼�������

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ӡ��̫����Ϣ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ:
 * ��  ��: 
 *   ʱ�� 2014��01��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void EthMonitor<Next, Succ>::PrintEthInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "-------------------------------------------------------";
	DLOG(INFO) << "### ETH || "
		       << GetMacStr(pkt_msg->l2_pkt_info.src_mac)
			   << "->" << GetMacStr(pkt_msg->l2_pkt_info.dst_mac)
			   << " | " << pkt_msg->l2_pkt_info.l3_prot
			   << " | " << pkt_msg->l2_pkt_info.l2_data_len;
}

}

#endif