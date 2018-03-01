/*#############################################################################
 * 文件名   : ip_frag_reassembler.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月02日
 * 文件描述 : Ipv4FragReassembler类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IPV4_FRAG_REASSEMBLER
#define BROADINTER_IPV4_FRAG_REASSEMBLER

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "message.hpp"
#include "tmas_typedef.hpp"
#include "ipv4_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * 描  述: IP分片重组器
 * 作  者: teck_zhou
 * 时  间: 2014年04月02日
 ******************************************************************************/
class Ipv4FragReassembler : public boost::noncopyable
{
public:
	Ipv4FragReassembler(Ipv4MonitorType* monitor, const PktMsg* pkt_msg);

	void ProcessFragPkt(const PktMsg* pkt_msg);

private:

	//--------------------------------------------------------------------------

	struct PktFrag
	{
		PktFrag(uint16 begin, const string str)
			: offset(begin), data(str) {}

		PktFrag(uint16 begin, const char* buf, uint16 len)
			: offset(begin), data(buf, len) {}

		uint16 offset;	// 分片在报文中的偏移
		string data;	// 分片数据
	};

	typedef std::list<PktFrag> PktFragList;
	typedef typename PktFragList::iterator FragIter;

	//--------------------------------------------------------------------------

private:

	// 所有分片是否都已经到齐
	bool IfPktCompleted() const;

	// 插入分片处理
	bool InsertPktFrag(const PktMsg* pkt_msg);

	// 处理首分片
	bool ProcFrontPktFrag(const PktMsg* pkt_msg);

	// 处理尾分片
	bool ProcRearPktFrag(const PktMsg* pkt_msg);

	// 第一个分片到来处理
	void ProcFirstInPktFrag(const PktMsg* pkt_msg);

	// 将分片插入到分片列表
	void DoInsertPktFrag(const PktMsg* pkt_msg);

	// 重组所有分片
	void ReasmPktFrags();

private:
	Ipv4MonitorType* ipv4_monitor_;

	Ipv4PktIdx pkt_idx_;	// 报文标识

	string header_;			// 从ETH头到IP头数据

	uint16 eth_hdr_len_;	// ETH头长度

	uint16 pkt_id_;			// 报文标识

	uint16 meat_;			// 实际到达的字节数

	uint16 pkt_end_;		// 报文的最后一个字节

	uint8 flag_;			// FRAG_FIRST_IN/FRAG_LAST_IN

	PktFragList pkt_frags_;	// 未合并的分片

	boost::mutex proc_mutex_;
};

}

#endif