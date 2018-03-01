/*#############################################################################
 * 文件名   : ip_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : Ipv4Monitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_IPV4_MONITOR_INL
#define BROADINTER_IPV4_MONITOR_INL

#include <glog/logging.h>

#include "ipv4_monitor.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "pkt_dispatcher.hpp"
#include "pkt_parser.hpp"
#include "tmas_config_parser.hpp"
#include "ipv4_frag_reassembler.hpp"

namespace BroadInter
{

#define MAX_IP_PKT_LEN	65535

const uint32 kIpv4FragCacheTimeout = 5;

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014年01月25日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
Ipv4Monitor<Next, Succ>::Ipv4Monitor() : enable_checksum_(false)
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数:
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool Ipv4Monitor<Next, Succ>::Init()
{
	GET_TMAS_CONFIG_BOOL("global.ip.enable-ip-checksum", enable_checksum_);

	frag_timer_.reset(new FreeTimer(boost::bind(&Ipv4Monitor<Next, Succ>::OnTick, this), 5));
	TMAS_ASSERT(frag_timer_);

	frag_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: IP层报文处理
 * 参  数: [in] msg_type 消息类型
 *		   [in] msg_data 消息数据
 * 返回值: 是否需要继续处理报文
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo Ipv4Monitor<Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	PktMsg* pkt_msg = static_cast<PktMsg*>(msg_data);

	// 只处理IP协议
	if (pkt_msg->l2_pkt_info.l3_prot != TMAS_ETH_TYPE_IP)
	{
		DLOG(WARNING) << "Unsupported L3 protocol "
			          << pkt_msg->l2_pkt_info.l3_prot;
		return PI_NOT_PROCESSED;
	}

	// 解析IP头信息
	if (!ParsePktIpInfo(enable_checksum_, pkt_msg)) 
	{
		return PI_HAS_PROCESSED;
	}

	PrintIpInfo(pkt_msg);

	// 只支持IPv4
	if (pkt_msg->l3_pkt_info.ip_ver != TMAS_IP_VER_4) 
	{
		return PI_NOT_PROCESSED;
	}

	// 源IP与目的IP一样的报文丢弃
	if (SRC_IP(pkt_msg) == DST_IP(pkt_msg)) 
	{
		DLOG(WARNING) << "Source is is the same as destination ip";
		return PI_HAS_PROCESSED;
	}

	// 非分片报文，直接往下传递继续处理
	if (!IsFragPkt(pkt_msg))
	{
		this->PassMsgToSuccProcessor(MSG_PKT, (void*)pkt_msg);
	}
	else // 分片报文，则需要进行重组
	{
		IncommingPktFrag(pkt_msg); 
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * 描  述: 分片报文处理
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void Ipv4Monitor<Next, Succ>::IncommingPktFrag(const PktMsg* pkt_msg)
{
	// 寻找分片所属重组器
	Ipv4FragReassemblerSP reassembler = GetFragReassembler(pkt_msg);
	if (!reassembler) return;
	
	reassembler->ProcessFragPkt(pkt_msg);
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取分片报文关联的分片重组器
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 分片重组器
 * 修  改:
 *   时间 2014年04月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
Ipv4FragReassemblerSP 
Ipv4Monitor<Next, Succ>::GetFragReassembler(const PktMsg* pkt_msg)
{
	//boost::mutex::scoped_lock lock(reassembler_mutex_);

	HashView& hash_view = reassemblers_.get<0>();
	auto iter = hash_view.find(Ipv4PktIdx(pkt_msg));
	if (iter != hash_view.end())
	{
		return iter->reassembler;
	}

	ReassemblerEntry entry(pkt_msg);
	entry.reassembler.reset(new Ipv4FragReassembler(this, pkt_msg));

	auto result = hash_view.insert(entry);
	if (!(result.second))
	{
		LOG(ERROR) << "Fail to insert reassembler entry";
		return nullptr;
	}

	return entry.reassembler;
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否是报文分片
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline bool Ipv4Monitor<Next, Succ>::IsFragPkt(const PktMsg* pkt_msg)
{
	return (pkt_msg->l3_pkt_info.mf_flag || pkt_msg->l3_pkt_info.frag_offset != 0);
}

/*-----------------------------------------------------------------------------
 * 描  述: 定时处理
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月18日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void Ipv4Monitor<Next, Succ>::OnTick()
{
	//boost::mutex::scoped_lock lock(reassembler_mutex_);

	uint32 time_now = GetMicroSecond();

	TimeView& time_view = reassemblers_.get<1>();
	for (auto iter = time_view.begin(); iter != time_view.end(); )
	{
		TMAS_ASSERT(iter->last_frag_in < time_now);
		if (time_now - iter->last_frag_in < kIpv4FragCacheTimeout) 
		{
			break;
		}
	
		iter = time_view.erase(iter);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 删除报文entry
 * 参  数: [in] pkt_entry 报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void Ipv4Monitor<Next, Succ>::RemoveReassembler(const Ipv4PktIdx& pkt_idx)
{
	DLOG(INFO) << "Remove reasmed ip packet entry";

	//boost::mutex::scoped_lock lock(reassembler_mutex_);
	
	HashView& hash_view = reassemblers_.get<0>();
	hash_view.erase(pkt_idx);
}

/*-----------------------------------------------------------------------------
 * 描  述: 打印IP层信息
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年01月22日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void Ipv4Monitor<Next, Succ>::PrintIpInfo(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "### IP || "
			  << to_string(pkt_msg->l3_pkt_info.src_ip)
			  << "->" << to_string(pkt_msg->l3_pkt_info.dst_ip)
	          << " | " << uint16(pkt_msg->l3_pkt_info.l4_prot)
	          << " | " << pkt_msg->l3_pkt_info.l3_data_len
			  << " | [" << uint16(pkt_msg->l3_pkt_info.mf_flag)
	          << ":" << pkt_msg->l3_pkt_info.pkt_id
			  << ":" << pkt_msg->l3_pkt_info.frag_offset << "]";
}

} // namespace BroadInter

#endif
