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
	: tcp_monitor_(tcp_monitor)
	, record_handshake_timeout_(false)
	, handshake_delay_(0)
{
	//��ȡ�����ļ���Ϣ
	GET_TMAS_CONFIG_INT("global.tcp.handshake-timeout", handshake_timeout_);

	if (handshake_timeout_ > 1024000)
	{
		handshake_timeout_ = 4000;
		LOG(WARNING) << "Invalid complete-handshake-timeout " 
			         << handshake_timeout_;
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
void TcpConnAnalyzer<Next, Succ>::ReInitialize()
{
	record_handshake_timeout_ = false;

	std::memset(&fsm_info_, 0x0, sizeof(fsm_info_));

	std::memset(&conn_stat_, 0x0, sizeof(conn_stat_));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��Ϣ���������
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��03��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
ProcInfo TcpConnAnalyzer<Next, Succ>::DoProcessMsg(MsgType msg_type, void* data)
{
	switch (msg_type)
	{
	case MSG_PKT:
		{
			PktMsg* pkt_msg = static_cast<PktMsg*>(data);
			PktMsgProc(pkt_msg);
		}
		break;

	case MSG_REINITIALIZE:
		ReInitialize();
		break; // ����Ϣ�������´���

	default:
		this->PassMsgToSuccProcessor(msg_type, data);
		break;
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������Ϣ����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::PktMsgProc(PktMsg* pkt_msg)
{
	// �����Ѿ��رջ��쳣�˳����ٴ�����
	if (fsm_info_.conn_state & (CONN_FAILED | CONN_CLOSED))
	{
		DLOG(WARNING) << "Unexpected packet | " << fsm_info_.conn_state;
		return; 
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
		this->PassMsgToSuccProcessor(MSG_PKT, (void*)pkt_msg);
	}
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
void TcpConnAnalyzer<Next, Succ>::ProcessTcpFsm(const PktMsg* pkt_msg)
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
void TcpConnAnalyzer<Next, Succ>::ProcHalfConnFsm(const PktMsg* pkt_msg, 
												  TcpFsmEvent fsm_event, 
												  HalfConnFsm& half_conn_fsm)
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
		DLOG(WARNING) << "Normally close connection | " << conn_id;

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
void TcpConnAnalyzer<Next, Succ>::CheckHandshakeTimeOut(const PktMsg* pkt_msg)
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

	// ���Ĳ�������������������ԭ����˳���Ѿ����ң�����Ȼ���ı�ǵ�ʱ��
	// �Ⱥ��ϵҲ����ʵ�ʲ�һ�£��������ʱ���Ǵ�С�쳣��������Ǿͼ�
	// ��ʱ���ǽ��е�����
	if (pkt_msg->arrive >= first_pkt_time_)
	{
		handshake_delay_ = (pkt_msg->arrive - first_pkt_time_) / 1000;
	}
	else
	{
		DLOG(INFO) << "Unordered time";
		handshake_delay_ = (first_pkt_time_ - pkt_msg->arrive) / 1000;
	}

	if (handshake_delay_ > handshake_timeout_)
	{
		ProcTcpHsTimeOut(pkt_msg->l4_pkt_info.conn_id);
	}

	conn_stat_.handshake_delay = handshake_delay_;

	fsm_info_.conn_state |= CONN_ESTABLISHED;

	record_handshake_timeout_ = true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼tcp���ֳ�ʱ������
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��06��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::ProcTcpHsTimeOut(const ConnId& conn_id)
{
	RecordHsTimeout(conn_id);

	tcp_monitor_->AbandonTcpConnection(conn_id);

	DLOG(WARNING) << "Abandon tcp connection for handshake timeout | " << conn_id;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼�������ֳ�ʱ
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::RecordHsTimeout(const ConnId& conn_id)
{
	TcpRecordInfo record_info;

	record_info.conn_id = conn_id;
	record_info.conn_event = TCE_HS_TIMEOUT;

	tcp_monitor_->GetTcpObserverManager().Process(record_info);
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
void TcpConnAnalyzer<Next, Succ>::ConnectionFailed(const ConnId& conn_id, 
	                                               ConnCloseReason reason)
{
	fsm_info_.half_conn_fsm[PKT_DIR_B2S].fsm_state = TFS_CLOSED;
	fsm_info_.half_conn_fsm[PKT_DIR_S2B].fsm_state = TFS_CLOSED;

	fsm_info_.conn_state |= CONN_FAILED;

	// TODO: ��¼�����쳣�ر���Ϣ
	LOG(WARNING) << "Connection failed(" << reason << ") | " << conn_id;

	RecordConnFailed(conn_id);

	DLOG(WARNING) << "Abandon failed tcp connection | " << conn_id;
	
	tcp_monitor_->AbandonTcpConnection(conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼�����쳣�ر�
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::RecordConnFailed(const ConnId& conn_id)
{
	TcpRecordInfo record_info;

	record_info.conn_id = conn_id;
	record_info.conn_event = TCE_CONN_ABORT;

	tcp_monitor_->GetTcpObserverManager().Process(record_info);
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
void TcpConnAnalyzer<Next, Succ>::ConnectionClosed(const ConnId& conn_id)
{
	fsm_info_.half_conn_fsm[PKT_DIR_B2S].fsm_state = TFS_CLOSED;
	fsm_info_.half_conn_fsm[PKT_DIR_S2B].fsm_state = TFS_CLOSED;

	fsm_info_.conn_state |= CONN_CLOSED;

	RecordConnClosed(conn_id);

	tcp_monitor_->RemoveTcpConnection(conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼���������ر�ʱ������ʱ����Ϣ
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::RecordConnClosed(const ConnId& conn_id)
{
	TcpRecordInfo record_info;

	record_info.conn_id = conn_id;
	record_info.hs_delay = handshake_delay_;
	record_info.conn_event = TCE_CONN_CLOSE;

	tcp_monitor_->GetTcpObserverManager().Process(record_info);
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
void TcpConnAnalyzer<Next, Succ>::ProcessConnStat(const PktMsg* pkt_msg)
{
	if (conn_stat_.total_pkt_num == 0)
	{
		first_pkt_time_ = pkt_msg->arrive;

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