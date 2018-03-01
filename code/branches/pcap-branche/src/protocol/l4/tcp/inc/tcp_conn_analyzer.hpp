/*#############################################################################
 * �ļ���   : tcp_conn_analyzer.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��09��
 * �ļ����� : TcpConnAnalyzer������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONN_ANALYZER
#define BROADINTER_TCP_CONN_ANALYZER

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "pkt_processor.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "tcp_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: TCP���ӷ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��10��
 ******************************************************************************/
template<class Next, class Succ>
class TcpConnAnalyzer : public PktProcessor<TcpConnAnalyzer<Next, Succ>, Next, Succ>
{
public:
	TcpConnAnalyzer(TcpMonitorType* tcp_monitor);

	//--- TODO: ���Ż���������

	// ������Ϣ����
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// ������Ϣ����
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* data);

private:
	//--------------------------------------------------------------------------

	struct HalfConnStat
	{
		HalfConnStat() : send_pkt_num(0), send_pkt_size(0) {}

		uint64 send_pkt_num;
		uint64 send_pkt_size;
	};

	//--------------------------------------------------------------------------

	struct TcpConnStat
	{
		TcpConnStat() : total_pkt_num(0), total_pkt_size(0), handshake_delay(0) {}

		uint64 total_pkt_num;
		uint64 total_pkt_size;
		uint32 handshake_delay;
		
		HalfConnStat half_conn_stat[PKT_DIR_BUTT];
	};

	//--------------------------------------------------------------------------

	struct HalfConnFsm
	{
		HalfConnFsm() : conn_role(TCR_UNKONWN), fsm_state(TFS_INIT) {}

		TcpConnRole conn_role;	// server or client
		TcpFsmState fsm_state;  // TCP״̬��״̬
	};

	//--------------------------------------------------------------------------

	struct TcpFsmInfo
	{
		TcpFsmInfo() : conn_state(0) {}

		uint8 conn_state;	// 0bit: ���ֳɹ� 1bit: ���ӽ���  2bit: ���ӹر�
		
		HalfConnFsm half_conn_fsm[PKT_DIR_BUTT];
	};

	//--------------------------------------------------------------------------

private:
	// ���³�ʼ��������֧�ֶ���ػ�
	inline void ReInitialize();

	// ����TCP״̬��
	inline void ProcessTcpFsm(const PktMsgSP& pkt_msg);

	// �����쳣��ֹ
	inline void ConnectionFailed(const ConnId& conn_id, ConnCloseReason reason);

	// ���������ر�
	inline void ConnectionClosed(const ConnId& conn_id);

	// ��������ӵ�TCP״̬��
	inline void ProcHalfConnFsm(const PktMsgSP& pkt_msg, 
								TcpFsmEvent fsm_event, 
								HalfConnFsm& half_conn_fsm);

	// �����������ͳ��
	inline void ProcessConnStat(const PktMsgSP& pkt_msg);

	// �鿴���ӵ����������Ƿ�ʱ
	inline void CheckHandshakeTimeOut(const PktMsgSP& pkt_msg);

	// ��¼�������ֳ�ʱ������
	inline void ProcTcpHsTimeOut(uint32 times, const ConnId& conn_id);

private:
	TcpMonitorType* tcp_monitor_;

	// ��һ�����Ĵ���ʱ��
	ptime first_pkt_time_;			

	// ����ͳ����Ϣ
	TcpConnStat conn_stat_;			

	// ���ӵ�״̬����Ϣ
	TcpFsmInfo fsm_info_;			

	//�Ƿ��¼�����ֳ�ʱʱ��
	bool record_handshake_timeout_; 

	// ���ֳ�ʱ��ֵ
	uint32 complete_handshake_timeout_; 
};

}

#include "tcp_conn_analyzer-inl.hpp"

#endif