/*#############################################################################
 * �ļ���   : tcp_conn_analyzer.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��09��
 * �ļ����� : TcpConnAnalyzer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_TCP_CONN_ANALYZER_INL
#define BROADINTER_TCP_CONN_ANALYZER_INL

#include <glog/logging.h>

#include "tcp_conn_analyzer.hpp"
#include "tcp_monitor.hpp"
#include "tmas_assert.hpp"
#include "tcp_fsm.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"

namespace BroadInter
{

#define CONN_STARTED        0x01
#define CONN_ESTABLISHED    0x02
#define CONN_CLOSED         0x04
#define CONN_FAILED         0x10

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] tcp_monitor TcpMonitor
 *         [in] conn_id ���ӱ�ʶ
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
TcpConnAnalyzer<Next, Succ>::TcpConnAnalyzer(TcpMonitorType* tcp_monitor) 
	: tcp_monitor_(tcp_monitor), record_handshake_timeout_(false)
{
	//��ȡ�����ļ���Ϣ
	GET_TMAS_CONFIG_INT("global.tcp.complete-handshake-timeout", 
		                complete_handshake_timeout_);

	if (complete_handshake_timeout_ > 1024)
	{
		complete_handshake_timeout_ = 4;
		LOG(WARNING) << "Invalid complete-handshake-timeout " 
			         << complete_handshake_timeout_;
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���³�ʼ��
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ReInitialize()
{
	record_handshake_timeout_ = false;

	std::memset(&fsm_info_, 0x0, sizeof(fsm_info_));

	std::memset(&conn_stat_, 0x0, sizeof(conn_stat_));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������Ϣ
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��03��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo TcpConnAnalyzer<Next, Succ>::DoProcessMsg(MsgType msg_type, void* data)
{
	if (msg_type == MSG_REINITIALIZE)
	{
		ReInitialize();

		// ����Ϣ�������´���
	}
	else
	{
		this->PassMsgToSuccProcessor(msg_type, data);
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo TcpConnAnalyzer<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	// �����Ѿ��رջ��쳣�˳����ٴ�����
	if (fsm_info_.conn_state & (CONN_FAILED | CONN_CLOSED))
	{
		DLOG(WARNING) << "Unexpected packet | " << fsm_info_.conn_state;
		return PI_HAS_PROCESSED; 
	}

	// ��������ͳ��
	ProcessConnStat(pkt_msg);

	// ����TCP״̬��
	ProcessTcpFsm(pkt_msg);

	// ���������ʱ�Ƿ����
	CheckHandshakeTimeOut(pkt_msg);

	// ���payload��Ϊ������Ҫ���ϼ�������
	if (pkt_msg->l4_pkt_info.l4_data_len != 0)
	{
		this->PassPktToSuccProcessor(pkt_msg);
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: TCPЭ��״̬������
 * ��  ��: [in] conn_info ������Ϣ
 *         [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ProcessTcpFsm(const PktMsgSP& pkt_msg)
{
	TMAS_ASSERT(SEND_DIR < PKT_DIR_BUTT);

	HalfConnFsm& send_half_fsm = fsm_info_.half_conn_fsm[SEND_DIR];
	HalfConnFsm& recv_half_fsm = fsm_info_.half_conn_fsm[RECV_DIR];

	// RST��־�����쳣�رգ�������״̬���ڲ�����
	if (RST_FLAG(pkt_msg))
	{
		DLOG(WARNING) << "Received a packet setting RST";
		ConnectionFailed(pkt_msg->l4_pkt_info.conn_id, CCR_RECV_RST);
		return;
	}
	
	// �ȴ����Ͷ�״̬��

	TcpFsmEvent fsm_event = TFE_NONE;
	fsm_event |= (SYN_FLAG(pkt_msg) ? TFE_SEND_SYN : TFE_NONE);
	fsm_event |= (ACK_FLAG(pkt_msg) ? TFE_SEND_ACK : TFE_NONE);
	fsm_event |= (FIN_FLAG(pkt_msg) ? TFE_SEND_FIN : TFE_NONE);

	ProcHalfConnFsm(pkt_msg, fsm_event, send_half_fsm);
	
	// �ٴ�����ն�״̬��

	fsm_event = TFE_NONE;
	fsm_event |= (SYN_FLAG(pkt_msg) ? TFE_RECV_SYN : TFE_NONE);
	fsm_event |= (ACK_FLAG(pkt_msg) ? TFE_RECV_ACK : TFE_NONE);
	fsm_event |= (FIN_FLAG(pkt_msg) ? TFE_RECV_FIN : TFE_NONE);

	ProcHalfConnFsm(pkt_msg, fsm_event, recv_half_fsm);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������FSM
 * ��  ��: [in] pkt_msg ������Ϣ
 *         [in] fsm_event ״̬���¼�
 *         [in][out] half_conn_fsm ������FSM
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ProcHalfConnFsm(const PktMsgSP& pkt_msg, 
	TcpFsmEvent fsm_event, HalfConnFsm& half_conn_fsm)
{
	if (half_conn_fsm.fsm_state == TFS_CLOSED) return;

	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	TcpFsmState next_state = TFS_INIT;
	if (!TcpFsm::ProcessTcpFsm(half_conn_fsm.conn_role, fsm_event, 
		                       half_conn_fsm.fsm_state, next_state))
	{
		LOG(ERROR) << "Fail to process fsm | " << conn_id;
		return ConnectionFailed(conn_id, CCR_ERROR);
	}
	
	half_conn_fsm.fsm_state = next_state;

	// �����ر�����
	if (next_state == TFS_CLOSED)
	{
		ConnectionClosed(conn_id);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ֳ�ʱ���
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::CheckHandshakeTimeOut(const PktMsgSP& pkt_msg)
{
	if (record_handshake_timeout_) return;

	TMAS_ASSERT(SEND_DIR < PKT_DIR_BUTT);
	
	HalfConnFsm& send_half_fsm = fsm_info_.half_conn_fsm[SEND_DIR];	
	HalfConnFsm& recv_half_fsm = fsm_info_.half_conn_fsm[RECV_DIR];

	if ((send_half_fsm.fsm_state != TFS_ESTABLISHED)
		|| (recv_half_fsm.fsm_state != TFS_ESTABLISHED))
	{
		return;
	}

	uint32 delay = GetDurationMilliSeconds(first_pkt_time_, MicroTimeNow());
	if (delay > complete_handshake_timeout_)
	{
		ProcTcpHsTimeOut(delay, pkt_msg->l4_pkt_info.conn_id);
	}

	conn_stat_.handshake_delay = delay;

	fsm_info_.conn_state |= CONN_ESTABLISHED;

	record_handshake_timeout_ = true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼tcp���ֳ�ʱ������
 * ��  ��: [in] times ��ʱʱ��
 *         [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��06��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ProcTcpHsTimeOut(uint32 times, const ConnId& conn_id)
{
	//��ʽ�������ʱ��Ϣ
	std::ofstream ofs("tcp_syn.log", std::ios_base::app);

  	ofs << GetLocalTime() << "   " << conn_id << "   " << times << std::endl;

	ofs.close();
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����쳣�ر�
 * ��  ��: [in] conn_info ������Ϣ
 *         [in] reason ���ӹر�ԭ��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ConnectionFailed(
	const ConnId& conn_id, ConnCloseReason reason)
{
	fsm_info_.half_conn_fsm[PKT_DIR_B2S].fsm_state = TFS_CLOSED;
	fsm_info_.half_conn_fsm[PKT_DIR_S2B].fsm_state = TFS_CLOSED;

	fsm_info_.conn_state |= CONN_FAILED;

	// TODO: ��¼�����쳣�ر���Ϣ
	LOG(WARNING) << "Connection failed(" << reason << ") | " << conn_id;
	
	tcp_monitor_->AbandonTcpConnection(conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������رմ���
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ConnectionClosed(const ConnId& conn_id)
{
	fsm_info_.half_conn_fsm[PKT_DIR_B2S].fsm_state = TFS_CLOSED;
	fsm_info_.half_conn_fsm[PKT_DIR_S2B].fsm_state = TFS_CLOSED;

	fsm_info_.conn_state |= CONN_CLOSED;

	tcp_monitor_->RemoveTcpConnection(conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������ͳ��
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpConnAnalyzer<Next, Succ>::ProcessConnStat(const PktMsgSP& pkt_msg)
{
	if (conn_stat_.total_pkt_num == 0)
	{
		first_pkt_time_ = MicroTimeNow();

		uint8 direction = pkt_msg->l3_pkt_info.direction;
		TMAS_ASSERT(direction < PKT_DIR_BUTT);

		fsm_info_.half_conn_fsm[direction].conn_role = TCR_CLIENT;
		fsm_info_.half_conn_fsm[(direction + 1) % 2].conn_role = TCR_SERVER;
	}

	conn_stat_.total_pkt_num++;
	conn_stat_.total_pkt_size += L4_DATA_LEN(pkt_msg);

	uint8 direction = pkt_msg->l3_pkt_info.direction;
	TMAS_ASSERT(direction < PKT_DIR_BUTT);

	conn_stat_.half_conn_stat[direction].send_pkt_num++;
	conn_stat_.half_conn_stat[direction].send_pkt_size += L4_DATA_LEN(pkt_msg);
}

}

#endif