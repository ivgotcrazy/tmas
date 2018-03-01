/*#############################################################################
 * 文件名   : udp_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : UdpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月10日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool UdpMonitor<Next, Succ>::Init()
{
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理UDP报文
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 处理结果
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo UdpMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	// 只处理UDP报文
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

	return PI_HAS_PROCESSED; // 处理链终止，上层处理继续
}

}

#endif