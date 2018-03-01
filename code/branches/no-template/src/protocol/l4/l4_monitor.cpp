/*#############################################################################
 * �ļ���   : l4_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : L4Monitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "l4_monitor.hpp"
#include "tmas_typedef.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "tmas_config_parser.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "pkt_resolver.hpp"

namespace BroadInter
{

#define TRANSPORT_TCP	0x6		// TCP protocol
#define TRANSPORT_UDP	0x11	// UDP protocol

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool L4Monitor::Init()
{
	TcpMonitorSP tcp_monitor(new TcpMonitor(shared_from_this()));
	if (!tcp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init tcp monitor";
		return false;
	}

	UdpMonitorSP udp_monitor(new UdpMonitor());

	tcp_monitor->set_successor(udp_monitor);

	pkt_processor_ = tcp_monitor;

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����㱨�Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
ProcInfo L4Monitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	TMAS_ASSERT(msg_data);
	TMAS_ASSERT(pkt_processor_);

	ProcInfo ret = pkt_processor_->Process(MSG_PKT, msg_data);
	
	if (ret == PI_L4_CONTINUE)
	{
		return PI_CHAIN_CONTINUE;
	}
	else
	{
		return PI_RET_STOP;
	}
}

}

