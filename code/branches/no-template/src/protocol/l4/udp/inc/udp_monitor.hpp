/*#############################################################################
 * �ļ���   : udp_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : UdpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_UDP_MONITOR
#define BROADINTER_UDP_MONITOR

#include <boost/noncopyable.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: UDP���Ĵ�����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class UdpMonitor : public PktProcessor
{
public:
	UdpMonitor();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	void ResolveTcpInfo(PktMsgSP& pkt_msg);

private:
	bool udp_switch_;

	PktProcessorSP pkt_processor_;
};

}

#endif