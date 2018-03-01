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
template<class Next, class Succ>
class UdpMonitor : public PktProcessor<UdpMonitor<Next, Succ>, Next, Succ>
{
public:
	UdpMonitor();

	bool Init();

	// ������Ϣ����
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

private:
	// udpЭ�鿪��
	bool udp_switch_;
};

}

#include "udp_monitor-inl.hpp"

#endif