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

class TcpMonitor;

/*******************************************************************************
 * ��  ��: TCP���ӷ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��10��
 ******************************************************************************/
class TcpConnAnalyzer : public PktProcessor
{
public:
	TcpConnAnalyzer(TcpMonitor* tcp_monitor);

	virtual void ReInit() override;

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

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
	void ProcessTcpFsm(const PktMsgSP& pkt_msg);

	void ConnectionFailed(const ConnId& conn_id, ConnCloseReason reason);
	void ConnectionClosed(const ConnId& conn_id);

	void ProcHalfConnFsm(const PktMsgSP& pkt_msg, 
		                 TcpFsmEvent fsm_event, 
						 HalfConnFsm& half_conn_fsm);
	void ProcessConnStat(const PktMsgSP& pkt_msg);

	void CheckHandshakeTimeOut(const PktMsgSP& pkt_msg);

	void ProcTcpHsTimeOut(uint32 times, const ConnId& conn_id);

private:
	TcpMonitor* tcp_monitor_;

	ptime first_pkt_time_;  // ��һ�����Ĵ���ʱ��

	TcpConnStat conn_stat_;	// ����ͳ��

	TcpFsmInfo fsm_info_;	// ״̬����Ϣ

	bool record_handshake_timeout_; //�Ƿ��¼�����ֳ�ʱʱ��

	uint32 complete_handshake_timeout_; 
};

}

#endif