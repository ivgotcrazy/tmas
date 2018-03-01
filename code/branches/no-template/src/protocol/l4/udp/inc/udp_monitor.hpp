/*#############################################################################
 * 文件名   : udp_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : UdpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: UDP报文处理类
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
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