/*#############################################################################
 * 文件名   : udp_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : UdpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月10日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 处理UDP报文
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 报文是否需要继续处理
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
ProcInfo UdpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);

	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// 只处理TCP报文，非TCP报文由本处理链其他处理器继续处理
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_UDP)
	{
		return PI_CHAIN_CONTINUE;
	}

	DLOG(INFO) << "Start process UDP packet";

	ResolveTcpInfo(pkt_msg);

	// process

	return PI_RET_STOP; // 处理链终止，上层处理继续
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析TCP信息
 * 参  数: [in] pkt_msg 报文
 * 返回值:
 * 修  改:
 *   时间 2014年01月22日
 *   作者 teck_zhou
 *   描述 创建
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