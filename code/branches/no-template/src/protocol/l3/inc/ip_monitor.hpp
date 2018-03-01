/*#############################################################################
 * 文件名   : ip_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : IpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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

	uint32 src_ip;	// 源IP
	uint32 dst_ip;	// 目的IP
	uint16 id;		// 报文标识
	uint8 prot;		// 协议类型
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
 * 描  述: IP数据分片重组
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
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

		uint16 offset;	  // 分片在报文中的偏移
		std::string data; // 分片数据
	};

	typedef std::list<Fragment> FragList;
	typedef FragList::iterator FragIter;

	struct IpPktEntry
	{
		IpPktEntry() : meat(0), pkt_end(0), flags(0) 
		{
			last_in_time = time(0);
		}

		std::string header_data;	// 从ETH头到IP头数据
		uint16 eth_hdr_len;			// ETH头长度
		uint16 pkt_id;				// 报文标识
		uint32 first_in_time;		// 第一个到达的分片的到达时间
		uint32 last_in_time;		// 最后一个报文的到达时间
		uint16 meat;				// 实际到达的字节数
		uint16 pkt_end;				// 报文的最后一个字节
		FragList frags;				// 未合并的分片
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
