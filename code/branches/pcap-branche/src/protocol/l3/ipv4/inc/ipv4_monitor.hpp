/*#############################################################################
 * �ļ���   : ipv4_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : Ipv4Monitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: IP���ݷ�Ƭ����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
template<class Next, class Succ>
class Ipv4Monitor : public PktProcessor<Ipv4Monitor<Next, Succ>, Next, Succ>
{
public:
	Ipv4Monitor();

	bool Init();
	
	//--- TODO: ���Ż���������

	// ������Ϣ����
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	void RemoveReassembler(const Ipv4PktIdx& pkt_idx);

private:

	//--------------------------------------------------------------------------

	struct ReassemblerEntry
	{
		ReassemblerEntry(const PktMsgSP& pkt_msg) 
			: pkt_idx(pkt_msg), last_frag_in(0) {}

		Ipv4PktIdx pkt_idx;					// ���ı�ʶ
		uint32 last_frag_in;				// ���һ����Ƭ����ʱ��
		Ipv4FragReassemblerSP reassembler;	// ��Ƭ������
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

	// ��ӡIP����Ϣ
	void PrintIpInfo(const PktMsgSP& pkt_msg);

	// �����Ƿ��Ƿ�Ƭ����
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
