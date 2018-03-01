/*#############################################################################
 * 文件名   : eth_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月21日
 * 文件描述 : EthMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_ETH_MONITOR_INL
#define BROADINTER_ETH_MONITOR_INL

#include <glog/logging.h>

#include "eth_monitor.hpp"
#include "pkt_processor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "pkt_parser.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年01月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool EthMonitor<Next, Succ>::Init()
{
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值:
 * 修  改: 
 *   时间 2014年01月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo EthMonitor<Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	PktMsg* pkt_msg = static_cast<PktMsg*>(msg_data);

	// 解析ETH头信息
	if (!ParsePktEthInfo(pkt_msg))
	{
		LOG(ERROR) << "Fail to resolve L2 info";
		return PI_HAS_PROCESSED;
	}

	PrintEthInfo(pkt_msg);

	this->PassMsgToSuccProcessor(MSG_PKT, (void*)pkt_msg); // 往下继续处理

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * 描  述: 打印以太层信息
 * 参  数: [in] pkt_msg 报文消息
 * 返回值:
 * 修  改: 
 *   时间 2014年01月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void EthMonitor<Next, Succ>::PrintEthInfo(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "-------------------------------------------------------";
	DLOG(INFO) << "### ETH || "
		       << GetMacStr(pkt_msg->l2_pkt_info.src_mac)
			   << "->" << GetMacStr(pkt_msg->l2_pkt_info.dst_mac)
			   << " | " << pkt_msg->l2_pkt_info.l3_prot
			   << " | " << pkt_msg->l2_pkt_info.l2_data_len;
}

}

#endif