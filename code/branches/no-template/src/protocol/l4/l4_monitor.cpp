/*#############################################################################
 * 文件名   : l4_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : L4Monitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 传输层报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 
 * 修  改: 
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
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

