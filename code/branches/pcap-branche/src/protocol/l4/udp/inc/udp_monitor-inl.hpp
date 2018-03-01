/*#############################################################################
 * �ļ���   : udp_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : UdpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_UDP_MONITOR_INL
#define BROADINTER_UDP_MONITOR_INL

#include <glog/logging.h>

#include "udp_monitor.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"
#include "pkt_parser.hpp"
#include "tmas_assert.hpp"


namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
UdpMonitor<Next, Succ>::UdpMonitor() : udp_switch_(false)
{
	GET_TMAS_CONFIG_BOOL("global.protocol.udp", udp_switch_);

	if (!udp_switch_)
	{
		DLOG(WARNING) << "UDP monitor is closed";
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool UdpMonitor<Next, Succ>::Init()
{
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����UDP����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo UdpMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	// ֻ����UDP����
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_UDP)
	{
		return PI_NOT_PROCESSED;
	}

	DLOG(INFO) << "Start process UDP packet";

	if (!ParsePktUdpInfo(pkt_msg))
	{
		LOG(ERROR) << "Fail to parse udp info";
		return PI_HAS_PROCESSED;
	}

	// process

	return PI_HAS_PROCESSED; // ��������ֹ���ϲ㴦�����
}

}

#endif