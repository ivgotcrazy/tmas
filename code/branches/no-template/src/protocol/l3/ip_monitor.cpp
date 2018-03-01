/*#############################################################################
 * 文件名   : ip_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : IpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "ip_monitor.hpp"
#include "pkt_resolver.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "pkt_dispatcher.hpp"
#include "pkt_parser.hpp"
#include "tmas_config_parser.hpp"

namespace BroadInter
{

#define FRAG_COMPLETE	4
#define FRAG_FIRST_IN	2
#define FRAG_LAST_IN	1

#define MAX_IP_PKT_LEN	65535

#define IP_FRAG_POLICY_FIRST	0
#define IP_FRAG_POLICY_LAST		1

const uint32 kMaxOverTime = 5;

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014年01月25日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
IpMonitor::IpMonitor() : enable_checksum_(false)
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
bool IpMonitor::Init()
{
	GET_TMAS_CONFIG_BOOL("global.ip.enable-ip-checksum", enable_checksum_);

	ip_timer_.reset(new FreeTimer(boost::bind(&IpMonitor::OnTick, this), 5));
	TMAS_ASSERT(ip_timer_);

	ip_timer_->Start();

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
ProcInfo IpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	TMAS_ASSERT(msg_data);

	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// 只处理IP协议
	if (pkt_msg->l2_pkt_info.l3_prot != TMAS_ETH_TYPE_IP)
	{
		DLOG(WARNING) << "L3 protocol " << pkt_msg->l2_pkt_info.l3_prot;
		return PI_RET_STOP;
	}

	// 解析IP头信息
	if (!ParsePktIpInfo(enable_checksum_, pkt_msg)) return PI_RET_STOP;

	PrintIpInfo(pkt_msg);

	// 只处理IPv4协议
	if (pkt_msg->l3_pkt_info.ip_ver != TMAS_IP_VER_4) return PI_RET_STOP;

	// 源IP与目的IP一样的报文丢弃
	if (SRC_IP(pkt_msg) == DST_IP(pkt_msg)) return PI_RET_STOP;

	// 非分片报文，直接往下传递继续处理
	if (!IsFragment(pkt_msg)) return PI_CHAIN_CONTINUE;

	// 分片报文，则需要进行重组
	IncomingFragment(pkt_msg); 
		
	// 重组完成的报文会自行往下传递，上层一律直接返回即可
	return PI_RET_STOP;
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
bool IpMonitor::IsFragment(const PktMsgSP& pkt_msg)
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
void IpMonitor::OnTick()
{
	boost::mutex::scoped_lock lock(frag_mutex_);

	uint32 time_now = time(0);
	IpPktEntryView& entry_view = ip_frags_.get<1>();
	FragPkMapEntryIter iter = entry_view.begin();
	
	for (; iter != entry_view.end(); iter++)
	{
		TMAS_ASSERT(iter->entry.last_in_time < time_now);
		if (time_now - iter->entry.last_in_time < kMaxOverTime) break;
	
		entry_view.erase(iter);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 替换索引容器中的PktEntry
 * 参  数: [in] pkt_msg 传入待处理报文
 *         [in] pkt_entry IpPktEntry
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IpMonitor::ReplaceEntry(const FragPktId& key, IpPktEntry& entry)
{
	FragIdView& frag_id_view = ip_frags_.get<0>();
	FragPkMapIter pkt_iter = frag_id_view.find(key);
	
	if (pkt_iter == ip_frags_.end()) return false;

	IpPktInfo info = *pkt_iter;
	
	if (!frag_id_view.modify(pkt_iter, PktEntryReplace(entry)))
	{
		LOG(ERROR) << "Fail to replace PktEntry ";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: fragment处理
 * 参  数: [in] pkt_msg 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void IpMonitor::IncomingFragment(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Incoming one IP fragment";

	boost::mutex::scoped_lock lock(frag_mutex_);

	// 查找分片所属报文
	FragPktId pkt_key(pkt_msg);
	IpPktEntry pkt_entry = FindPktEntry(pkt_key);

	// 将分片插入列表
	if (!InsertFragment(pkt_entry, pkt_msg))
	{
		LOG(ERROR) << "Fail to insert fragment";
		return;
	}

	//重新插入索引容器中
	if (!ReplaceEntry(pkt_key, pkt_entry))
	{
		LOG(ERROR) << "Fail to replace entry ";
		return;
	}

	// 如果所有分片都已经到达，则重组分片
	if (IsPktCompleted(pkt_entry))
	{
		ReasmIpFrags(pkt_entry);
		RemovePktEntry(pkt_key);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文重组
 * 参  数: [in] pkt_entry entry
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void IpMonitor::ReasmIpFrags(const IpPktEntry& pkt_entry)
{
	DLOG(INFO) << "Reasm ip fragments to packet";

	PktEntry pkt;

	pkt.len = pkt_entry.header_data.length() + pkt_entry.meat;
	pkt.buf = new char[pkt.len];

	char* copy_begin = const_cast<char*>(pkt.buf);

	std::memcpy(copy_begin, pkt_entry.header_data.data(), pkt_entry.header_data.length());

	copy_begin += pkt_entry.header_data.length();

	for (const Fragment& frag : pkt_entry.frags)
	{
		std::memcpy(copy_begin, frag.data.data(), frag.data.length());
		copy_begin += frag.data.length();
	}

	char* ip_hdr = const_cast<char*>(pkt_entry.header_data.data()) + pkt_entry.eth_hdr_len;

	// 刷新IP数据总长度
	PR::SetIpTotalLen(ip_hdr, pkt_entry.meat);

	// 刷新分片信息
	PR::SetMfFlag(ip_hdr, false);
	PR::SetFragOffset(ip_hdr, 0);
	
	// 当做刚捕获的报文进行处理
	PktDispatcher::GetInstance().ProcessPacket(pkt);

	delete [] pkt.buf; // TODO: 这里的内存申请需要优化
}

/*-----------------------------------------------------------------------------
 * 描  述: 删除报文entry
 * 参  数: [in] pkt_id 报文标识
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void IpMonitor::RemovePktEntry(const FragPktId& pkt_id)
{
	DLOG(INFO) << "Remove reasmed ip packet entry";

	ip_frags_.erase(pkt_id);
}

/*-----------------------------------------------------------------------------
 * 描  述: 向报文中添加分片
 * 参  数: [in] pkt_entry 报文entry
 *         [in] pkt_msg 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IpMonitor::InsertFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Start to insert one fragment to packet";

	if (L3_DATA_LEN(pkt_msg) == 0)
	{
		LOG(ERROR) << "Zero l3 data length";
		return false;
	}

	// 校验分片偏移和分片长度的合法性
	if ((FRAG_OFFSET(pkt_msg) + L3_DATA_LEN(pkt_msg)) > MAX_IP_PKT_LEN)
	{
		LOG(ERROR) << "Invalid fragment offset: " << FRAG_OFFSET(pkt_msg)
			       << " or len: " << L3_DATA_LEN(pkt_msg);
		return false;
	}

	// 首分片和尾分片需要做特殊处理
	if (pkt_msg->l3_pkt_info.frag_offset == 0)
	{
		if (!ProcFrontFragment(pkt_entry, pkt_msg))
		{
			DLOG(WARNING) << "Fail to process front fragment";
			return false;
		}
	}
	else if (!pkt_msg->l3_pkt_info.mf_flag)
	{
		if (!ProcRearFragment(pkt_entry, pkt_msg))
		{
			DLOG(WARNING) << "Fail to process rear fragment";
			return false;
		}
	}

	// 如果这是第一个到达的报文，直接插入即可
	if (pkt_entry.frags.empty())
	{
		ProcFirstInFragment(pkt_entry, pkt_msg);
	}
	else
	{
		DoInsertFragment(pkt_entry, pkt_msg);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 向报文中添加分片
 * 参  数: [in] pkt_entry 报文entry
 *         [in] pkt_msg 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月25日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void IpMonitor::DoInsertFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	Fragment incoming_frag(FRAG_OFFSET(pkt_msg), 
		                   pkt_msg->l4_pkt_info.l4_hdr, 
						   L3_DATA_LEN(pkt_msg));

	FragIter left = pkt_entry.frags.end();

	for (auto iter = pkt_entry.frags.begin(); iter != pkt_entry.frags.end(); )
	{
		uint16 iter_begin = iter->offset;
		uint16 iter_end = iter->offset + iter->data.length();

		uint16 incoming_begin = incoming_frag.offset;
		uint16 incoming_end = incoming_frag.offset + incoming_frag.data.length();

		// iter在incoming的左边
		if (iter_end <= incoming_begin)
		{
			break; // 不用再往后处理了
		}
		// iter在incoming的右边
		else if (iter_begin >= incoming_end)
		{
			left = iter;	
		}
		// iter包含incoming
		else if (iter_begin <= incoming_begin && iter_end >= incoming_end)
		{
			return; // 丢弃incoming fragment
		}
		// incoming包含iter
		else if (incoming_begin <= iter_begin && incoming_end >= iter_end)
		{
			iter = pkt_entry.frags.erase(iter);
			continue;
		}
		// 左相交
		else if (iter_begin < incoming_begin && iter_end < incoming_end)
		{
			iter->data.erase(incoming_begin, iter_end - incoming_begin);
		}
		// 右相交
		else if (iter_begin > incoming_begin && iter_end > incoming_end)
		{
			iter->offset = incoming_end;
			iter->data.erase(0, incoming_end - iter_begin);
			left = iter;
		}
		else // 其他
		{
			LOG(ERROR) << "Unexpected condition";
			return;
		}

		++iter;
	}

	pkt_entry.meat += pkt_msg->l3_pkt_info.l3_data_len;

	pkt_entry.frags.insert(left, incoming_frag);
}

/*-----------------------------------------------------------------------------
 * 描  述: 第一个到达分片处理
 * 参  数: [in] pkt_entry 报文entry
 *         [in] pkt_msg 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void IpMonitor::ProcFirstInFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	pkt_entry.first_in_time = time(0);

	pkt_entry.meat = pkt_msg->l3_pkt_info.l3_data_len;

	Fragment frag(pkt_msg->l3_pkt_info.frag_offset, 
		pkt_msg->l4_pkt_info.l4_hdr, 
		pkt_msg->l3_pkt_info.l3_data_len);

	pkt_entry.frags.push_back(frag);
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理首分片
 * 参  数: [in] pkt_entry 报文entry
 *         [in] pkt_msg 传入待处理报文
 * 返回值: 是否继续处理
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IpMonitor::ProcFrontFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	// 第一次收到首分片
	if (!(pkt_entry.flags & FRAG_FIRST_IN))
	{
		pkt_entry.flags |= FRAG_FIRST_IN;

		pkt_entry.pkt_id = pkt_msg->l3_pkt_info.pkt_id;

		pkt_entry.eth_hdr_len = pkt_msg->l2_pkt_info.eth_hdr_len;

		// 拷贝ETH头到IP头所有数据
		pkt_entry.header_data.append(pkt_msg->pkt.buf, 
			pkt_msg->l4_pkt_info.l4_hdr - pkt_msg->pkt.buf);

		return true;
	}

	// 如果是收到重复的首分片，需要根据配置策略处理
	// TODO: 当前默认使用丢弃策略，后续优化

	DLOG(WARNING) << "Received repeated front fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理尾分片
 * 参  数: [in] pkt_entry 报文entry
 *         [in] pkt_msg 传入待处理报文
 * 返回值: 是否继续处理
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IpMonitor::ProcRearFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	// 第一次收到最后一个分片
	if (!(pkt_entry.flags & FRAG_LAST_IN))
	{
		pkt_entry.flags |= FRAG_LAST_IN;

		pkt_entry.pkt_end = pkt_msg->l3_pkt_info.frag_offset + 
			pkt_msg->l3_pkt_info.l3_data_len;

		return true;
	}

	// 收到重复的尾分片，需要根据配置策略处理
	// TODO: 当前默认使用丢弃策略，后续优化

	DLOG(WARNING) << "Received repeated rear fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否所有的分片都已经到达
 * 参  数: [in] pkt_entry 报文entry
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IpMonitor::IsPktCompleted(const IpPktEntry& pkt_entry) const
{
	return (pkt_entry.meat == pkt_entry.pkt_end) 
		&& (pkt_entry.flags & FRAG_FIRST_IN)
		&& (pkt_entry.flags & FRAG_LAST_IN);
}

/*-----------------------------------------------------------------------------
 * 描  述: fragment处理
 * 参  数: [in] pkt_id 报文标识
 *         [in] frag 分片信息
 *         [out] pkt 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
IpMonitor::IpPktEntry IpMonitor::FindPktEntry(const FragPktId& pkt_id)
{
	FragIdView& frag_id_view = ip_frags_.get<0>();
	FragPkMapIter pkt_iter = frag_id_view.find(pkt_id);
	
	if (pkt_iter != ip_frags_.end())
	{
		return (*pkt_iter).entry;
	}
	else
	{
		InsertResult result = ip_frags_.insert(
			FragPkMap::value_type(pkt_id, IpPktEntry()));

		return (*(result.first)).entry;
	}
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
void IpMonitor::PrintIpInfo(const PktMsgSP& pkt_msg)
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

}
