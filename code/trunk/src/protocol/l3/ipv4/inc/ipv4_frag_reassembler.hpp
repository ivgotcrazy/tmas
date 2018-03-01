/*#############################################################################
 * �ļ���   : ip_frag_reassembler.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��02��
 * �ļ����� : Ipv4FragReassembler������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: IP��Ƭ������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��04��02��
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

		uint16 offset;	// ��Ƭ�ڱ����е�ƫ��
		string data;	// ��Ƭ����
	};

	typedef std::list<PktFrag> PktFragList;
	typedef typename PktFragList::iterator FragIter;

	//--------------------------------------------------------------------------

private:

	// ���з�Ƭ�Ƿ��Ѿ�����
	bool IfPktCompleted() const;

	// �����Ƭ����
	bool InsertPktFrag(const PktMsg* pkt_msg);

	// �����׷�Ƭ
	bool ProcFrontPktFrag(const PktMsg* pkt_msg);

	// ����β��Ƭ
	bool ProcRearPktFrag(const PktMsg* pkt_msg);

	// ��һ����Ƭ��������
	void ProcFirstInPktFrag(const PktMsg* pkt_msg);

	// ����Ƭ���뵽��Ƭ�б�
	void DoInsertPktFrag(const PktMsg* pkt_msg);

	// �������з�Ƭ
	void ReasmPktFrags();

private:
	Ipv4MonitorType* ipv4_monitor_;

	Ipv4PktIdx pkt_idx_;	// ���ı�ʶ

	string header_;			// ��ETHͷ��IPͷ����

	uint16 eth_hdr_len_;	// ETHͷ����

	uint16 pkt_id_;			// ���ı�ʶ

	uint16 meat_;			// ʵ�ʵ�����ֽ���

	uint16 pkt_end_;		// ���ĵ����һ���ֽ�

	uint8 flag_;			// FRAG_FIRST_IN/FRAG_LAST_IN

	PktFragList pkt_frags_;	// δ�ϲ��ķ�Ƭ

	boost::mutex proc_mutex_;
};

}

#endif