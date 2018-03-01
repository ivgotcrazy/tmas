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
template<class Next, class Succ>
class TcpConnReorder : public PktProcessor<TcpConnReorder<Next, Succ>, Next, Succ>
{
public:
	TcpConnReorder(TcpMonitorType* tcp_monitor);
	~TcpConnReorder();

	//--- TODO: ���Ż���������

	// ��Ϣ����
	ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data);

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

	typedef std::list<PktMsg*> PktMsgList;
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
	void PktMsgProc(PktMsg* pkt_msg);

	// ���³�ʼ��������֧�ֶ���ػ�
	void Reinitialize();

	// ���³�ʼ��������״̬������֧�ֶ���ػ�
	void ReInitHalfConn(HalfConnInfo& half_conn);

	// �жϱ����Ƿ�����
	PktOrderType GetPktOrderType(const PktMsg* pkt_msg);

	// �Ƿ���TCP�������ֵĵ�һ������
	bool IsFirstPktOf3WHS(const PktMsg* pkt_msg);

	// �Ƿ���TCP�������ֵĵڶ�������
	bool IsSecondPktOf3WHS(const PktMsg* pkt_msg);

	// ������������
	void ProcessOrderedPkt(PktMsg* pkt_msg, bool cache_pkt);

	// �����汨��
	void TryProcCachedPkt();

	// �ȴ��������ģ������������汨��
	void TryProcOrderedPkt(PktMsg* pkt_msg);

	// ����������
	void ProcessUnorderedPkt(PktMsg* pkt_msg);

	// �������Ļ�������
	void AddUnorderedPkt(const PktMsg* pkt_msg);

	// ������һ�������ĺ���Ҫˢ������״̬
	void RefreshTcpConn(const PktMsg* pkt_msg);

	// ����һ������Ļ��汨��
	bool ProcOneCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter);

	// �������Ĳ��뻺������
	void InsertUnorderedPktToList(const PktMsg* pkt_msg, PktMsgList& pkt_list);

	bool FindInsertPos(const PktMsg* pkt_msg, PktMsgList& pkt_list, PktMsgIter& pkt_iter);

	// ����ĳһ�������ӵĻ��汨��
	void ProcHalfConnCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter,
		bool& local_half_flag, bool& other_half_flag);

	// �ع�TCP���ӣ�ʹ���������ܹ���������
	void RefactorTcpConn();

	// ͨ��ˢ������״̬������ʹ�û��汨�ı������
	void MakeChacedPktOrdered();

	// ��ȡ˫�����ӻ��汨������
	uint32 GetTotalCachedPktNum();

	// ����ʧ�ܴ���
	void HandshakeFailed(const PktMsg* pkt_msg);

private:
	TcpMonitorType* tcp_monitor_;

	// �ظ����ļ�⡢���������������״̬��Ϣ
	TcpConnInfo conn_info_;

	// ��֤ͬһʱ��ֻ��һ���̴߳���ĳһ��TCP���ӵı��ġ�
	//boost::mutex reorder_mutex_;

	// һ�����Ӳ��ܻ���̫��������
	uint32 max_cached_unordered_pkt_;

	// �����ʧ���������ֱ��ģ������ع�������
	uint32 max_cached_pkt_before_handshake_;

	// ���ܳ������ӹرպ󣬻��к���������Ҫ��������
	// ���ܶ���̶߳����������Ӵ�����ڣ�����һ���߳�
	// �����ӹر��ˣ������߳��ٽ��룬��Ӧ���ٴ�����
	bool conn_closed_;
};

}

#include "tcp_conn_reorder-inl.hpp"

#endif

