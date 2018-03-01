/*#############################################################################
 * 文件名   : ipv4_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : Ipv4Monitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IPV4_MONITOR
#define BROADINTER_IPV4_MONITOR

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
#include "ipv4_typedef.hpp"
#include "ipv4_frag_reassembler.hpp"

namespace BroadInter
{

using std::string;

using namespace boost::multi_index;

/*******************************************************************************
 * 描  述: IP数据分片重组
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
template<class Next, class Succ>
class Ipv4Monitor : public PktProcessor<Ipv4Monitor<Next, Succ>, Next, Succ>
{
public:
	Ipv4Monitor();

	bool Init();
	
	//--- TODO: 待优化访问限制

	// 报文消息处理
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	void RemoveReassembler(const Ipv4PktIdx& pkt_idx);

private:

	//--------------------------------------------------------------------------

	struct ReassemblerEntry
	{
		ReassemblerEntry(const PktMsgSP& pkt_msg) 
			: pkt_idx(pkt_msg), last_frag_in(0) {}

		Ipv4PktIdx pkt_idx;					// 报文标识
		uint32 last_frag_in;				// 最后一个分片到达时间
		Ipv4FragReassemblerSP reassembler;	// 分片重组器
	};

	//--------------------------------------------------------------------------
	
	typedef multi_index_container<
		ReassemblerEntry,
		indexed_by<
			hashed_unique<
				member<ReassemblerEntry, Ipv4PktIdx, &ReassemblerEntry::pkt_idx>
			>,
			ordered_non_unique<
				member<ReassemblerEntry, uint32, &ReassemblerEntry::last_frag_in>
			>
		> 
	>ReassemblerContainer;

	typedef typename ReassemblerContainer::template nth_index<0>::type HashView;
	typedef typename ReassemblerContainer::template nth_index<1>::type TimeView;

	//--------------------------------------------------------------------------

	class LastFragInModifier
	{
	public:
		LastFragInModifier(uint32 new_time) : new_time_(new_time){}

	    void operator()(ReassemblerEntry& reassembler_entry)  
	    {  
	        reassembler_entry.last_frag_in = new_time_; 
	    }

	private:
		uint32 new_time_;
	};

	//--------------------------------------------------------------------------

private:
	void OnTick();

	void IncommingPktFrag(const PktMsgSP& pkt_msg);

	Ipv4FragReassemblerSP GetFragReassembler(const PktMsgSP& pkt_msg);

	// 打印IP层信息
	void PrintIpInfo(const PktMsgSP& pkt_msg);

	// 报文是否是分片报文
	inline bool IsFragPkt(const PktMsgSP& pkt_msg);

private:
	ReassemblerContainer reassemblers_;

	boost::mutex reassembler_mutex_;

	boost::scoped_ptr<FreeTimer> frag_timer_;

	bool enable_checksum_;
};

}

#include "ipv4_monitor-inl.hpp"

#endif
