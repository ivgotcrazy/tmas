/*#############################################################################
 * �ļ���   : udp_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : UdpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "udp_monitor.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"
#include "pkt_resolver.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

#define UDP_HDR_LEN 16

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
UdpMonitor::UdpMonitor() : udp_switch_(false)
{
	GET_TMAS_CONFIG_BOOL("global.protocol.udp", udp_switch_);

	if (!udp_switch_)
	{
		DLOG(WARNING) << "UDP monitor is closed";
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����UDP����
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: �����Ƿ���Ҫ��������
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
ProcInfo UdpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);

	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// ֻ����TCP���ģ���TCP�����ɱ�������������������������
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_UDP)
	{
		return PI_CHAIN_CONTINUE;
	}

	DLOG(INFO) << "Start process UDP packet";

	ResolveTcpInfo(pkt_msg);

	// process

	return PI_RET_STOP; // ��������ֹ���ϲ㴦�����
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����TCP��Ϣ
 * ��  ��: [in] pkt_msg ����
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��01��22��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void UdpMonitor::ResolveTcpInfo(PktMsgSP& pkt_msg)
{
	pkt_msg->l4_pkt_info.src_port = PR::GetUdpSrcPortH(pkt_msg->l4_pkt_info.l4_hdr);
	pkt_msg->l4_pkt_info.dst_port = PR::GetUdpDstPortH(pkt_msg->l4_pkt_info.l4_hdr);

	pkt_msg->l4_pkt_info.conn_id = ConnId(IPPROTO_UDP,
										 pkt_msg->l3_pkt_info.src_ip,
										 pkt_msg->l4_pkt_info.src_port,
										 pkt_msg->l3_pkt_info.dst_ip,
										 pkt_msg->l4_pkt_info.dst_port);

	L4_DATA_LEN(pkt_msg) = L3_DATA_LEN(pkt_msg) - UDP_HDR_LEN;

	L7_DATA(pkt_msg) = L4_HDR(pkt_msg) + UDP_HDR_LEN;
}

}