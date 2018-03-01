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

	// ������Ϣ����
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// �Ǳ�����Ϣ����
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* data);

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
	// ���³�ʼ��������֧�ֶ���ػ�
	inline void Reinitialize();

	// ���³�ʼ��������״̬������֧�ֶ���ػ�
	inline void ReInitHalfConn(HalfConnInfo& half_conn);

	// �жϱ����Ƿ�����
	inline PktOrderType GetPktOrderType(const PktMsgSP& pkt_msg);

	// �Ƿ���TCP�������ֵĵ�һ������
	inline bool IsFirstPktOf3WHS(const PktMsgSP& pkt_msg);

	// �Ƿ���TCP�������ֵĵڶ�������
	inline bool IsSecondPktOf3WHS(const PktMsgSP& pkt_msg);

	// ������������
	inline void ProcessOrderedPkt(PktMsgSP& pkt_msg);

	// �����汨��
	inline void TryProcCachedPkt();

	// �ȴ��������ģ������������汨��
	inline void TryProcOrderedPkt(PktMsgSP& pkt_msg);

	// ����������
	inline void ProcessUnorderedPkt(PktMsgSP& pkt_msg);

	// �������Ļ�������
	inline void AddUnorderedPkt(const PktMsgSP& pkt_msg);

	// ������һ�������ĺ���Ҫˢ������״̬
	inline void RefreshTcpConn(const PktMsgSP& pkt_msg);

	// ����һ������Ļ��汨��
	inline bool ProcOneCachedPkt(PktMsgList& pkt_list, 
		                         PktMsgIter& pkt_iter);

	// �������Ĳ��뻺������
	inline void InsertUnorderedPktToList(const PktMsgSP& pkt_msg, 
										 PktMsgList& pkt_list);

	// ����ĳһ�������ӵĻ��汨��
	inline void ProcHalfConnCachedPkt(PktMsgList& pkt_list, 
									  PktMsgIter& pkt_iter,
									  bool& local_half_flag, 
									  bool& other_half_flag);

	// �ع�TCP���ӣ�ʹ���������ܹ���������
	inline void RefactorTcpConn();

	// ͨ��ˢ������״̬������ʹ�û��汨�ı������
	inline void MakeChacedPktOrdered();

	// ��ȡ˫�����ӻ��汨������
	inline uint32 GetTotalCachedPktNum();

	// ����ʧ�ܴ���
	inline void HandshakeFailed(const PktMsgSP& pkt_msg);

private:
	TcpMonitorType* tcp_monitor_;

	// �ظ����ļ�⡢���������������״̬��Ϣ
	TcpConnInfo conn_info_;

	// ��֤ͬһʱ��ֻ��һ���̴߳���ĳһ��TCP���ӵı��ġ�
	boost::mutex reorder_mutex_;

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

