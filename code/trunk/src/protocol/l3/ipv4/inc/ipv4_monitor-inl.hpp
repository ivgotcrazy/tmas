/*#############################################################################
 * �ļ���   : ip_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : Ipv4Monitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ���캯��
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��01��25��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
Ipv4Monitor<Next, Succ>::Ipv4Monitor() : enable_checksum_(false)
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��:
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: IP�㱨�Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *		   [in] msg_data ��Ϣ����
 * ����ֵ: �Ƿ���Ҫ����������
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo Ipv4Monitor<Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	PktMsg* pkt_msg = static_cast<PktMsg*>(msg_data);

	// ֻ����IPЭ��
	if (pkt_msg->l2_pkt_info.l3_prot != TMAS_ETH_TYPE_IP)
	{
		DLOG(WARNING) << "Unsupported L3 protocol "
			          << pkt_msg->l2_pkt_info.l3_prot;
		return PI_NOT_PROCESSED;
	}

	// ����IPͷ��Ϣ
	if (!ParsePktIpInfo(enable_checksum_, pkt_msg)) 
	{
		return PI_HAS_PROCESSED;
	}

	PrintIpInfo(pkt_msg);

	// ֻ֧��IPv4
	if (pkt_msg->l3_pkt_info.ip_ver != TMAS_IP_VER_4) 
	{
		return PI_NOT_PROCESSED;
	}

	// ԴIP��Ŀ��IPһ���ı��Ķ���
	if (SRC_IP(pkt_msg) == DST_IP(pkt_msg)) 
	{
		DLOG(WARNING) << "Source is is the same as destination ip";
		return PI_HAS_PROCESSED;
	}

	// �Ƿ�Ƭ���ģ�ֱ�����´��ݼ�������
	if (!IsFragPkt(pkt_msg))
	{
		this->PassMsgToSuccProcessor(MSG_PKT, (void*)pkt_msg);
	}
	else // ��Ƭ���ģ�����Ҫ��������
	{
		IncommingPktFrag(pkt_msg); 
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��Ƭ���Ĵ���
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void Ipv4Monitor<Next, Succ>::IncommingPktFrag(const PktMsg* pkt_msg)
{
	// Ѱ�ҷ�Ƭ����������
	Ipv4FragReassemblerSP reassembler = GetFragReassembler(pkt_msg);
	if (!reassembler) return;
	
	reassembler->ProcessFragPkt(pkt_msg);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ��Ƭ���Ĺ����ķ�Ƭ������
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ��Ƭ������
 * ��  ��:
 *   ʱ�� 2014��04��02��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �Ƿ��Ǳ��ķ�Ƭ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline bool Ipv4Monitor<Next, Succ>::IsFragPkt(const PktMsg* pkt_msg)
{
	return (pkt_msg->l3_pkt_info.mf_flag || pkt_msg->l3_pkt_info.frag_offset != 0);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʱ����
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��18��
 *   ���� tom_liu
 *   ���� ����
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
 * ��  ��: ɾ������entry
 * ��  ��: [in] pkt_entry ����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ��ӡIP����Ϣ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��22��
 *   ���� teck_zhou
 *   ���� ����
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
