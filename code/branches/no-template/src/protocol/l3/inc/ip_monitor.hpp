/*#############################################################################
 * �ļ���   : ip_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : IpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IP_MONITOR
#define BROADINTER_IP_MONITOR

#include <list>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "message.hpp"
#include "timer.hpp"

namespace BroadInter
{

struct FragPktId
{
	FragPktId(uint8 protocol, uint32 src, uint32 dst, uint16 seq) 
		: src_ip(src), dst_ip(dst), id(seq), prot(protocol) 
	{
	}

	FragPktId(const PktMsgSP& pkt_msg)
		: src_ip(pkt_msg->l3_pkt_info.src_ip)
		, dst_ip(pkt_msg->l3_pkt_info.dst_ip)
		, id(pkt_msg->l3_pkt_info.pkt_id)
		, prot(pkt_msg->l2_pkt_info.l3_prot)
	{
	}

	bool operator==(const FragPktId& pkt_id) const
	{
		return (src_ip == pkt_id.src_ip) 
			&& (dst_ip == pkt_id.dst_ip)
			&& (id == pkt_id.id)
			&& (prot == pkt_id.prot);
	}

	uint32 src_ip;	// ԴIP
	uint32 dst_ip;	// Ŀ��IP
	uint16 id;		// ���ı�ʶ
	uint8 prot;		// Э������
};

struct FragPktIdHash
{
	std::size_t operator()(const FragPktId& index) const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, index.src_ip);
		boost::hash_combine(seed, index.dst_ip);
		boost::hash_combine(seed, index.id);
		boost::hash_combine(seed, index.prot);
		return seed;
	}
};

inline std::size_t hash_value(const FragPktId& pkt_id)
{
	size_t seed = 0;
	boost::hash_combine(seed, boost::hash_value(pkt_id.prot));
	boost::hash_combine(seed, boost::hash_value(pkt_id.src_ip));
	boost::hash_combine(seed, boost::hash_value(pkt_id.dst_ip));
	boost::hash_combine(seed, boost::hash_value(pkt_id.id));

	return seed;
}

using namespace boost::multi_index;

/*******************************************************************************
 * ��  ��: IP���ݷ�Ƭ����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class IpMonitor : public PktProcessor
{
public:
	IpMonitor();
	bool Init();
	
private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:

	struct Fragment
	{
		Fragment(uint16 begin, const std::string str)
			: offset(begin), data(str) 
		{
		}

		Fragment(uint16 begin, const char* buf, uint16 len)
			: offset(begin), data(buf, len) 
		{
		}

		uint16 offset;	  // ��Ƭ�ڱ����е�ƫ��
		std::string data; // ��Ƭ����
	};

	typedef std::list<Fragment> FragList;
	typedef FragList::iterator FragIter;

	struct IpPktEntry
	{
		IpPktEntry() : meat(0), pkt_end(0), flags(0) 
		{
			last_in_time = time(0);
		}

		std::string header_data;	// ��ETHͷ��IPͷ����
		uint16 eth_hdr_len;			// ETHͷ����
		uint16 pkt_id;				// ���ı�ʶ
		uint32 first_in_time;		// ��һ������ķ�Ƭ�ĵ���ʱ��
		uint32 last_in_time;		// ���һ�����ĵĵ���ʱ��
		uint16 meat;				// ʵ�ʵ�����ֽ���
		uint16 pkt_end;				// ���ĵ����һ���ֽ�
		FragList frags;				// δ�ϲ��ķ�Ƭ
		uint8 flags;				// FRAG_FIRST_IN/FRAG_LAST_IN/FRAG_COMPLETE

		bool operator < (const IpPktEntry& entry) const
		{	
			return last_in_time >= entry.last_in_time;
		}
	};

	struct IpPktInfo
	{
		IpPktInfo(FragPktId s_id, IpPktEntry s_entry): id(s_id), entry(s_entry){}
		FragPktId id;
		IpPktEntry entry;
	};
	
	typedef multi_index_container<
		IpPktInfo, 
		indexed_by<
			hashed_unique<
				member<IpPktInfo, FragPktId, &IpPktInfo::id>,
				FragPktIdHash
			>,
			ordered_non_unique<
				member<IpPktInfo, IpPktEntry, &IpPktInfo::entry> 
			>
		> 
	>FragPkMap;

	class PktEntryReplace
	{
	public:
		PktEntryReplace(const IpPktEntry& entry) : pkt_entry(entry){}  
	    void operator()(IpPktInfo& info)  
	    {  
	        info.entry = pkt_entry;  
	    }

	private:
		IpPktEntry pkt_entry;
	};

	typedef FragPkMap::iterator FragPkMapIter;
	typedef FragPkMap::nth_index<1>::type::iterator FragPkMapEntryIter;
	typedef FragPkMap::nth_index<0>::type FragIdView;
	typedef FragPkMap::nth_index<1>::type IpPktEntryView;
	
	//typedef boost::unordered_map<FragPktId, IpPktEntry> FragPkMap;
	typedef std::pair<FragPkMap::iterator, bool> InsertResult;

	struct InsertFrag
	{
		Fragment frag;
		FragIter left;
	};

	typedef std::vector<InsertFrag> InsertFragVec;

private:
	void OnTick();
	void IncomingFragment(const PktMsgSP& pkt_msg);
	bool ReplaceEntry(const FragPktId& key, IpPktEntry& entry);
	IpPktEntry FindPktEntry(const FragPktId& pkt_id);
	bool InsertFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg);
	bool IsPktCompleted(const IpPktEntry& pkt_entry) const;
	void ReasmIpFrags(const IpPktEntry& pkt_entry);
	void RemovePktEntry(const FragPktId& pkt_id);
	void PrintIpInfo(const PktMsgSP& pkt_msg);
	bool IsFragment(const PktMsgSP& pkt_msg);
	bool ProcFrontFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg);
	bool ProcRearFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg);
	void ProcFirstInFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg);
	FragIter FindLeftFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg);
	void DoInsertFragment(IpPktEntry& pkt_entry, const PktMsgSP& pkt_msg);

private:
	FragPkMap ip_frags_;

	boost::mutex frag_mutex_;

	boost::scoped_ptr<FreeTimer> ip_timer_;

	bool enable_checksum_;
};

}

#endif
