/*#############################################################################
 * �ļ���   : tcp_conn_reorder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��09��
 * �ļ����� : TcpConnReorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONN_REORDER
#define BROADINTER_TCP_CONN_REORDER

#include <list>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "tcp_fsm.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: TCP������������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��10��
 ******************************************************************************/
class TcpConnReorder : public PktProcessor
{
public:
	TcpConnReorder(TcpMonitor* tcp_monitor);
	~TcpConnReorder();

	virtual void ReInit() override;

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	//--------------------------------------------------------------------------
	enum PktOrderType
	{
		POT_IN_ORDER,
		POT_OUT_OF_ORDER,
		POT_IGNORE,
		POT_RETRANSMIT
	};
	//--------------------------------------------------------------------------
	typedef std::list<PktMsgSP> PktMsgList;
	typedef PktMsgList::iterator PktMsgIter;
	//--------------------------------------------------------------------------
	struct HalfConnInfo
	{
		HalfConnInfo() : sent_seq_num(0), sent_ack_num(0)
			, next_seq_num(0), next_ack_num(0), send_pkt_num(0)
		{
		}

		uint32 sent_seq_num; // �ѷ��͵ķ��ͺ�
		uint32 sent_ack_num; // �ѷ��͵�ȷ�Ϻ�

		uint32 next_seq_num; // ��һ�����ĵķ��ͺ�
		uint32 next_ack_num; // ��һ�����ĵ�ȷ�Ϻ�

		uint64 send_pkt_num;
		PktMsgList cached_pkt; // �����Ļ���
	};
	//--------------------------------------------------------------------------
	struct TcpConnInfo
	{
		TcpConnInfo() : started(false) {}

		bool started; // �������ֵĵ�һ�������Ƿ��Ѿ�����

		HalfConnInfo half_conn[2]; // ���������������Ϣ
	};
	//--------------------------------------------------------------------------

private:
	PktOrderType GetPktOrderType(const PktMsgSP& pkt_msg);
	bool IsFirstPktOf3WHS(const PktMsgSP& pkt_msg);
	bool IsSecondPktOf3WHS(const PktMsgSP& pkt_msg);
	void TryProcOrderedPkt(const PktMsgSP& pkt_msg);
	void ProcessOrderedPkt(const PktMsgSP& pkt_msg);
	void ProcessUnorderedPkt(const PktMsgSP& pkt_msg);
	void TryProcCachedPkt();
	void AddUnorderedPkt(const PktMsgSP& pkt_msg);
	bool ProcOneCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter);
	void RefreshTcpConn(const PktMsgSP& pkt_msg);
	void ProcHalfConnCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter,
		                       bool& local_half_flag, bool& other_half_flag);
	void InsertUnorderedPktToList(const PktMsgSP& pkt_msg, PktMsgList& pkt_list);
	void ReInitHalfConn(HalfConnInfo& half_conn);

private:
	TcpMonitor* tcp_monitor_;

	TcpConnInfo conn_info_;

	// ����������֤ͬһʱ��ֻ��һ���߳�
	// �ڴ����TCP���ӵı��ġ�
	boost::mutex reorder_mutex_;

	uint32 max_cached_unordered_pkt_;
	uint32 max_cached_pkt_before_handshake_;
};

}

#endif

