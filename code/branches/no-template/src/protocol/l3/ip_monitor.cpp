/*#############################################################################
 * �ļ���   : ip_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : IpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ���캯��
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��01��25��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
IpMonitor::IpMonitor() : enable_checksum_(false)
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
bool IpMonitor::Init()
{
	GET_TMAS_CONFIG_BOOL("global.ip.enable-ip-checksum", enable_checksum_);

	ip_timer_.reset(new FreeTimer(boost::bind(&IpMonitor::OnTick, this), 5));
	TMAS_ASSERT(ip_timer_);

	ip_timer_->Start();

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
ProcInfo IpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	TMAS_ASSERT(msg_data);

	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// ֻ����IPЭ��
	if (pkt_msg->l2_pkt_info.l3_prot != TMAS_ETH_TYPE_IP)
	{
		DLOG(WARNING) << "L3 protocol " << pkt_msg->l2_pkt_info.l3_prot;
		return PI_RET_STOP;
	}

	// ����IPͷ��Ϣ
	if (!ParsePktIpInfo(enable_checksum_, pkt_msg)) return PI_RET_STOP;

	PrintIpInfo(pkt_msg);

	// ֻ����IPv4Э��
	if (pkt_msg->l3_pkt_info.ip_ver != TMAS_IP_VER_4) return PI_RET_STOP;

	// ԴIP��Ŀ��IPһ���ı��Ķ���
	if (SRC_IP(pkt_msg) == DST_IP(pkt_msg)) return PI_RET_STOP;

	// �Ƿ�Ƭ���ģ�ֱ�����´��ݼ�������
	if (!IsFragment(pkt_msg)) return PI_CHAIN_CONTINUE;

	// ��Ƭ���ģ�����Ҫ��������
	IncomingFragment(pkt_msg); 
		
	// ������ɵı��Ļ��������´��ݣ��ϲ�һ��ֱ�ӷ��ؼ���
	return PI_RET_STOP;
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
bool IpMonitor::IsFragment(const PktMsgSP& pkt_msg)
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
 * ��  ��: �滻���������е�PktEntry
 * ��  ��: [in] pkt_msg �����������
 *         [in] pkt_entry IpPktEntry
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: fragment����
 * ��  ��: [in] pkt_msg �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void IpMonitor::IncomingFragment(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Incoming one IP fragment";

	boost::mutex::scoped_lock lock(frag_mutex_);

	// ���ҷ�Ƭ��������
	FragPktId pkt_key(pkt_msg);
	IpPktEntry pkt_entry = FindPktEntry(pkt_key);

	// ����Ƭ�����б�
	if (!InsertFragment(pkt_entry, pkt_msg))
	{
		LOG(ERROR) << "Fail to insert fragment";
		return;
	}

	//���²�������������
	if (!ReplaceEntry(pkt_key, pkt_entry))
	{
		LOG(ERROR) << "Fail to replace entry ";
		return;
	}

	// ������з�Ƭ���Ѿ�����������Ƭ
	if (IsPktCompleted(pkt_entry))
	{
		ReasmIpFrags(pkt_entry);
		RemovePktEntry(pkt_key);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: [in] pkt_entry entry
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
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

	// ˢ��IP�����ܳ���
	PR::SetIpTotalLen(ip_hdr, pkt_entry.meat);

	// ˢ�·�Ƭ��Ϣ
	PR::SetMfFlag(ip_hdr, false);
	PR::SetFragOffset(ip_hdr, 0);
	
	// �����ղ���ı��Ľ��д���
	PktDispatcher::GetInstance().ProcessPacket(pkt);

	delete [] pkt.buf; // TODO: ������ڴ�������Ҫ�Ż�
}

/*-----------------------------------------------------------------------------
 * ��  ��: ɾ������entry
 * ��  ��: [in] pkt_id ���ı�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void IpMonitor::RemovePktEntry(const FragPktId& pkt_id)
{
	DLOG(INFO) << "Remove reasmed ip packet entry";

	ip_frags_.erase(pkt_id);
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
bool IpMonitor::InsertFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Start to insert one fragment to packet";

	if (L3_DATA_LEN(pkt_msg) == 0)
	{
		LOG(ERROR) << "Zero l3 data length";
		return false;
	}

	// У���Ƭƫ�ƺͷ�Ƭ���ȵĺϷ���
	if ((FRAG_OFFSET(pkt_msg) + L3_DATA_LEN(pkt_msg)) > MAX_IP_PKT_LEN)
	{
		LOG(ERROR) << "Invalid fragment offset: " << FRAG_OFFSET(pkt_msg)
			       << " or len: " << L3_DATA_LEN(pkt_msg);
		return false;
	}

	// �׷�Ƭ��β��Ƭ��Ҫ�����⴦��
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

	// ������ǵ�һ������ı��ģ�ֱ�Ӳ��뼴��
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
 * ��  ��: ��������ӷ�Ƭ
 * ��  ��: [in] pkt_entry ����entry
 *         [in] pkt_msg �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��25��
 *   ���� teck_zhou
 *   ���� ����
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
			iter = pkt_entry.frags.erase(iter);
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

	pkt_entry.meat += pkt_msg->l3_pkt_info.l3_data_len;

	pkt_entry.frags.insert(left, incoming_frag);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��һ�������Ƭ����
 * ��  ��: [in] pkt_entry ����entry
 *         [in] pkt_msg �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �����׷�Ƭ
 * ��  ��: [in] pkt_entry ����entry
 *         [in] pkt_msg �����������
 * ����ֵ: �Ƿ��������
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool IpMonitor::ProcFrontFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	// ��һ���յ��׷�Ƭ
	if (!(pkt_entry.flags & FRAG_FIRST_IN))
	{
		pkt_entry.flags |= FRAG_FIRST_IN;

		pkt_entry.pkt_id = pkt_msg->l3_pkt_info.pkt_id;

		pkt_entry.eth_hdr_len = pkt_msg->l2_pkt_info.eth_hdr_len;

		// ����ETHͷ��IPͷ��������
		pkt_entry.header_data.append(pkt_msg->pkt.buf, 
			pkt_msg->l4_pkt_info.l4_hdr - pkt_msg->pkt.buf);

		return true;
	}

	// ������յ��ظ����׷�Ƭ����Ҫ�������ò��Դ���
	// TODO: ��ǰĬ��ʹ�ö������ԣ������Ż�

	DLOG(WARNING) << "Received repeated front fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����β��Ƭ
 * ��  ��: [in] pkt_entry ����entry
 *         [in] pkt_msg �����������
 * ����ֵ: �Ƿ��������
 * ��  ��:
 *   ʱ�� 2014��01��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool IpMonitor::ProcRearFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg)
{
	// ��һ���յ����һ����Ƭ
	if (!(pkt_entry.flags & FRAG_LAST_IN))
	{
		pkt_entry.flags |= FRAG_LAST_IN;

		pkt_entry.pkt_end = pkt_msg->l3_pkt_info.frag_offset + 
			pkt_msg->l3_pkt_info.l3_data_len;

		return true;
	}

	// �յ��ظ���β��Ƭ����Ҫ�������ò��Դ���
	// TODO: ��ǰĬ��ʹ�ö������ԣ������Ż�

	DLOG(WARNING) << "Received repeated rear fragment";

	return false;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ����еķ�Ƭ���Ѿ�����
 * ��  ��: [in] pkt_entry ����entry
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool IpMonitor::IsPktCompleted(const IpPktEntry& pkt_entry) const
{
	return (pkt_entry.meat == pkt_entry.pkt_end) 
		&& (pkt_entry.flags & FRAG_FIRST_IN)
		&& (pkt_entry.flags & FRAG_LAST_IN);
}

/*-----------------------------------------------------------------------------
 * ��  ��: fragment����
 * ��  ��: [in] pkt_id ���ı�ʶ
 *         [in] frag ��Ƭ��Ϣ
 *         [out] pkt �����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ��ӡIP����Ϣ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��22��
 *   ���� teck_zhou
 *   ���� ����
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
