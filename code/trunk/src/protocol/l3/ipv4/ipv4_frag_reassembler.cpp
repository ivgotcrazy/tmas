/*#############################################################################
 * �ļ���   : ip_frag_reassembler.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��02��
 * �ļ����� : Ipv4FragReassembler��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ���캯��
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��04��02��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �����Ƭ����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��04��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void Ipv4FragReassembler::ProcessFragPkt(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "Incoming one ip fragment";

	boost::mutex::scoped_lock lock(proc_mutex_);

	// ����Ƭ���뵽��Ƭ���У�Ȼ���ٴ�������
	if (!InsertPktFrag(pkt_msg))
	{
		LOG(ERROR) << "Fail to insert fragment";
		return;
	}

	if (!IfPktCompleted()) return;

	// ������з�Ƭ���Ѿ�����������Ƭ

	ReasmPktFrags();

	ipv4_monitor_->RemoveReassembler(pkt_idx_);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ����еķ�Ƭ���Ѿ�����
 * ��  ��: 
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��04��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::IfPktCompleted() const
{
	return (meat_ == pkt_end_) && (flag_ & FRAG_FIRST_IN) && (flag_ & FRAG_LAST_IN);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������ӷ�Ƭ
 * ��  ��: [in] pkt_entry ����entry
 *         [in] pkt_msg �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::InsertPktFrag(const PktMsg* pkt_msg)
{
	if (L3_DATA_LEN(pkt_msg) == 0)
	{
		LOG(ERROR) << "Zero l3 data length";
		return false;
	}

	// У���Ƭƫ�ƺͷ�Ƭ���ȵĺϷ���
	if ((FRAG_OFFSET(pkt_msg) + L3_DATA_LEN(pkt_msg)) > MAX_IP_PKT_LEN)
	{
		LOG(ERROR) << "Invalid fragment offset: " 
			       << FRAG_OFFSET(pkt_msg)
			       << " or len: " << L3_DATA_LEN(pkt_msg);
		return false;
	}

	// �׷�Ƭ��β��Ƭ��Ҫ�����⴦��
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

	// ������ǵ�һ������ı��ģ�ֱ�Ӳ��뼴��
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
 * ��  ��: �����׷�Ƭ
 * ��  ��: [in] pkt_msg �����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::ProcFrontPktFrag(const PktMsg* pkt_msg)
{
	// ��һ���յ��׷�Ƭ
	if (!(flag_ & FRAG_FIRST_IN))
	{
		flag_ |= FRAG_FIRST_IN;

		pkt_id_ = pkt_msg->l3_pkt_info.pkt_id;

		eth_hdr_len_ = pkt_msg->l2_pkt_info.eth_hdr_len;

		// ����ETHͷ��IPͷ��������
		header_.append(pkt_msg->pkt.buf,
			pkt_msg->l4_pkt_info.l4_hdr - pkt_msg->pkt.buf);

		return true;
	}

	// ������յ��ظ����׷�Ƭ����Ҫ�������ò��Դ���
	// TODO: ��ǰĬ��ʹ�ö������ԣ������Ż�

	LOG(WARNING) << "Received repeated front fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����β��Ƭ
 * ��  ��: [in] pkt_msg �����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool Ipv4FragReassembler::ProcRearPktFrag(const PktMsg* pkt_msg)
{
	// ��һ���յ����һ����Ƭ
	if (!(flag_ & FRAG_LAST_IN))
	{
		flag_ |= FRAG_LAST_IN;

		pkt_end_ = pkt_msg->l3_pkt_info.frag_offset + 
			pkt_msg->l3_pkt_info.l3_data_len;

		return true;
	}

	// �յ��ظ���β��Ƭ����Ҫ�������ò��Դ���
	// TODO: ��ǰĬ��ʹ�ö������ԣ������Ż�

	LOG(WARNING) << "Received repeated rear fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������ӷ�Ƭ
 * ��  ��: [in] pkt_msg �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��25��
 *   ���� teck_zhou
 *   ���� ����
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

		// iter��incoming�����
		if (iter_end <= incoming_begin)
		{
			break; // ��������������
		}
		// iter��incoming���ұ�
		else if (iter_begin >= incoming_end)
		{
			left = iter;	
		}
		// iter����incoming
		else if (iter_begin <= incoming_begin && iter_end >= incoming_end)
		{
			return; // ����incoming fragment
		}
		// incoming����iter
		else if (incoming_begin <= iter_begin && incoming_end >= iter_end)
		{
			iter = pkt_frags_.erase(iter);
			continue;
		}
		// ���ཻ
		else if (iter_begin < incoming_begin && iter_end < incoming_end)
		{
			iter->data.erase(incoming_begin, iter_end - incoming_begin);
		}
		// ���ཻ
		else if (iter_begin > incoming_begin && iter_end > incoming_end)
		{
			iter->offset = incoming_end;
			iter->data.erase(0, incoming_end - iter_begin);
			left = iter;
		}
		else // ����
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
 * ��  ��: ��һ�������Ƭ����
 * ��  ��: [in] pkt_msg �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���鱨�ĵ����з�Ƭ
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void Ipv4FragReassembler::ReasmPktFrags()
{
	DLOG(INFO) << "Reasm ip fragments to packet";

	PktMsg pkt_msg;

	// ���䱨���ڴ�
	pkt_msg.pkt.len = header_.length() + meat_;
	pkt_msg.pkt.buf = new char[pkt_msg.pkt.len];

	pkt_msg.arrive = GetMicroSecond();

	char* copy_begin = pkt_msg.pkt.buf;

	// ����ETH->IP����ͷ
	std::memcpy(copy_begin, header_.c_str(), header_.length());

	copy_begin += header_.length();

	// ������Ƭ����
	for (const PktFrag& frag : pkt_frags_)
	{
		std::memcpy(copy_begin, frag.data.data(), frag.data.length());
		copy_begin += frag.data.length();
	}

	char* ip_hdr = const_cast<char*>(header_.data()) + eth_hdr_len_;

	// ˢ��IP�����ܳ���
	PR::SetIpTotalLen(ip_hdr, meat_);

	// ˢ�·�Ƭ��Ϣ
	PR::SetMfFlag(ip_hdr, false);
	PR::SetFragOffset(ip_hdr, 0);
	
	DLOG(WARNING) << "Reassemble packet";

	ipv4_monitor_->PassMsgToSuccProcessor(MSG_PKT, (void*)&pkt_msg);
	
	delete [] pkt_msg.pkt.buf; // TODO: ������ڴ�������Ҫ�Ż�
}

}