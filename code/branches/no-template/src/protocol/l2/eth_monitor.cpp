/*#############################################################################
 * 文件名   : eth_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月21日
 * 文件描述 : EthMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "eth_monitor.hpp"
#include "pkt_processor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "tmas_io.hpp"
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
bool EthMonitor::Init()
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
ProcInfo EthMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);

	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// 解析L2信息
	if (!ParsePktEthInfo(pkt_msg))
	{
		LOG(ERROR) << "Fail to resolve L2 info";
		return PI_RET_STOP;
	}

	PrintEthInfo(pkt_msg);

	return PI_CHAIN_CONTINUE; // 报文继续往下处理
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
void EthMonitor::PrintEthInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "-------------------------------------------------------";
	DLOG(INFO) << "### ETH || "
		       << GetMacStr(pkt_msg->l2_pkt_info.src_mac)
			   << "->" << GetMacStr(pkt_msg->l2_pkt_info.dst_mac)
			   << " | " << pkt_msg->l2_pkt_info.l3_prot
			   << " | " << pkt_msg->l2_pkt_info.l2_data_len;
}

}
