/*#############################################################################
 * 文件名   : ip_frag_reassembler.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月02日
 * 文件描述 : Ipv4FragReassembler类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "ipv4_frag_reassembler.hpp"
#include "pkt_dispatcher.hpp"
#include "pkt_resolver.hpp"
#include "ipv4_monitor.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "http_monitor.hpp"
#include "message.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

#define FRAG_FIRST_IN	0x2
#define FRAG_LAST_IN	0x1

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] pkt_msg 报文消息
 * 返回值:
 * 修  改:
 *   时间 2014年04月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
Ipv4FragReassembler::Ipv4FragReassembler(Ipv4MonitorType* monitor, const PktMsg* pkt_msg)
	: ipv4_monitor_(monitor)
	, pkt_idx_(pkt_msg)
	, eth_hdr_len_(0)
	, pkt_id_(0)
	, meat_(0)
	, flag_(0)
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理分片报文
 * 参  数: [in] pkt_msg 报文消息
 * 返回值:
 * 修  改:
 *   时间 2014年04月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void Ipv4FragReassembler::ProcessFragPkt(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "Incoming one ip fragment";

	boost::mutex::scoped_lock lock(proc_mutex_);

	// 将分片插入到分片队列，然后再处理重组
	if (!InsertPktFrag(pkt_msg))
	{
		LOG(ERROR) << "Fail to insert fragment";
		return;
	}

	if (!IfPktCompleted()) return;

	// 如果所有分片都已经到达，则重组分片

	ReasmPktFrags();

	ipv4_monitor_->RemoveReassembler(pkt_idx_);
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否所有的分片都已经到达
 * 参  数: 
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年04月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::IfPktCompleted() const
{
	return (meat_ == pkt_end_) && (flag_ & FRAG_FIRST_IN) && (flag_ & FRAG_LAST_IN);
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
bool Ipv4FragReassembler::InsertPktFrag(const PktMsg* pkt_msg)
{
	if (L3_DATA_LEN(pkt_msg) == 0)
	{
		LOG(ERROR) << "Zero l3 data length";
		return false;
	}

	// 校验分片偏移和分片长度的合法性
	if ((FRAG_OFFSET(pkt_msg) + L3_DATA_LEN(pkt_msg)) > MAX_IP_PKT_LEN)
	{
		LOG(ERROR) << "Invalid fragment offset: " 
			       << FRAG_OFFSET(pkt_msg)
			       << " or len: " << L3_DATA_LEN(pkt_msg);
		return false;
	}

	// 首分片和尾分片需要做特殊处理
	if (pkt_msg->l3_pkt_info.frag_offset == 0)
	{
		if (!ProcFrontPktFrag(pkt_msg))
		{
			LOG(WARNING) << "Fail to process front fragment";
			return false;
		}
	}
	else if (!pkt_msg->l3_pkt_info.mf_flag)
	{
		if (!ProcRearPktFrag(pkt_msg))
		{
			LOG(WARNING) << "Fail to process rear fragment";
			return false;
		}
	}

	// 如果这是第一个到达的报文，直接插入即可
	if (pkt_frags_.empty())
	{
		ProcFirstInPktFrag(pkt_msg);
	}
	else
	{
		DoInsertPktFrag(pkt_msg);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理首分片
 * 参  数: [in] pkt_msg 传入待处理报文
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::ProcFrontPktFrag(const PktMsg* pkt_msg)
{
	// 第一次收到首分片
	if (!(flag_ & FRAG_FIRST_IN))
	{
		flag_ |= FRAG_FIRST_IN;

		pkt_id_ = pkt_msg->l3_pkt_info.pkt_id;

		eth_hdr_len_ = pkt_msg->l2_pkt_info.eth_hdr_len;

		// 拷贝ETH头到IP头所有数据
		header_.append(pkt_msg->pkt.buf,
			pkt_msg->l4_pkt_info.l4_hdr - pkt_msg->pkt.buf);

		return true;
	}

	// 如果是收到重复的首分片，需要根据配置策略处理
	// TODO: 当前默认使用丢弃策略，后续优化

	LOG(WARNING) << "Received repeated front fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理尾分片
 * 参  数: [in] pkt_msg 传入待处理报文
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::ProcRearPktFrag(const PktMsg* pkt_msg)
{
	// 第一次收到最后一个分片
	if (!(flag_ & FRAG_LAST_IN))
	{
		flag_ |= FRAG_LAST_IN;

		pkt_end_ = pkt_msg->l3_pkt_info.frag_offset + 
			pkt_msg->l3_pkt_info.l3_data_len;

		return true;
	}

	// 收到重复的尾分片，需要根据配置策略处理
	// TODO: 当前默认使用丢弃策略，后续优化

	LOG(WARNING) << "Received repeated rear fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * 描  述: 向报文中添加分片
 * 参  数: [in] pkt_msg 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月25日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void Ipv4FragReassembler::DoInsertPktFrag(const PktMsg* pkt_msg)
{
	PktFrag incoming_frag(FRAG_OFFSET(pkt_msg), 
		                   pkt_msg->l4_pkt_info.l4_hdr, 
						   L3_DATA_LEN(pkt_msg));

	auto left = pkt_frags_.end();

	for (auto iter = pkt_frags_.begin(); iter != pkt_frags_.end(); )
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
			iter = pkt_frags_.erase(iter);
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

	meat_ += pkt_msg->l3_pkt_info.l3_data_len;

	pkt_frags_.insert(left, incoming_frag);
}

/*-----------------------------------------------------------------------------
 * 描  述: 第一个到达分片处理
 * 参  数: [in] pkt_msg 传入待处理报文
 * 返回值: 
 * 修  改:
 *   时间 2014年01月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void Ipv4FragReassembler::ProcFirstInPktFrag(const PktMsg* pkt_msg)
{
	meat_ = pkt_msg->l3_pkt_info.l3_data_len;

	PktFrag frag(pkt_msg->l3_pkt_info.frag_offset, 
		pkt_msg->l4_pkt_info.l4_hdr, 
		pkt_msg->l3_pkt_info.l3_data_len);

	pkt_frags_.push_back(frag);
}

/*-----------------------------------------------------------------------------
 * 描  述: 重组报文的所有分片
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void Ipv4FragReassembler::ReasmPktFrags()
{
	DLOG(INFO) << "Reasm ip fragments to packet";

	PktMsg pkt_msg;

	// 分配报文内存
	pkt_msg.pkt.len = header_.length() + meat_;
	pkt_msg.pkt.buf = new char[pkt_msg.pkt.len];

	pkt_msg.arrive = GetMicroSecond();

	char* copy_begin = pkt_msg.pkt.buf;

	// 拷贝ETH->IP报文头
	std::memcpy(copy_begin, header_.c_str(), header_.length());

	copy_begin += header_.length();

	// 拷贝分片数据
	for (const PktFrag& frag : pkt_frags_)
	{
		std::memcpy(copy_begin, frag.data.data(), frag.data.length());
		copy_begin += frag.data.length();
	}

	char* ip_hdr = const_cast<char*>(header_.data()) + eth_hdr_len_;

	// 刷新IP数据总长度
	PR::SetIpTotalLen(ip_hdr, meat_);

	// 刷新分片信息
	PR::SetMfFlag(ip_hdr, false);
	PR::SetFragOffset(ip_hdr, 0);
	
	DLOG(WARNING) << "Reassemble packet";

	ipv4_monitor_->PassMsgToSuccProcessor(MSG_PKT, (void*)&pkt_msg);
	
	delete [] pkt_msg.pkt.buf; // TODO: 这里的内存申请需要优化
}

}