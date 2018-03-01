/*#############################################################################
 * �ļ���   : eth_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��21��
 * �ļ����� : EthMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "eth_monitor.hpp"
#include "pkt_processor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "tmas_io.hpp"
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
bool EthMonitor::Init()
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
ProcInfo EthMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);

	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// ����L2��Ϣ
	if (!ParsePktEthInfo(pkt_msg))
	{
		LOG(ERROR) << "Fail to resolve L2 info";
		return PI_RET_STOP;
	}

	PrintEthInfo(pkt_msg);

	return PI_CHAIN_CONTINUE; // ���ļ������´���
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
void EthMonitor::PrintEthInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "-------------------------------------------------------";
	DLOG(INFO) << "### ETH || "
		       << GetMacStr(pkt_msg->l2_pkt_info.src_mac)
			   << "->" << GetMacStr(pkt_msg->l2_pkt_info.dst_mac)
			   << " | " << pkt_msg->l2_pkt_info.l3_prot
			   << " | " << pkt_msg->l2_pkt_info.l2_data_len;
}

}
