/*#############################################################################
 * �ļ���   : tcp_conn_analyzer.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��09��
 * �ļ����� : TcpConnAnalyzer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_conn_analyzer.hpp"
#include "tcp_monitor.hpp"
#include "tmas_assert.hpp"
#include "tcp_fsm.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"

#include <boost/date_time/local_time_adjustor.hpp>

namespace BroadInter
{
#define CONN_STARTED		0x01
#define CONN_ESTABLISHED	0x02
#define CONN_CLOSED			0x04
#define CONN_FAILED			0x10

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
TcpConnAnalyzer::TcpConnAnalyzer(TcpMonitor* tcp_monitor) 
	: tcp_monitor_(tcp_monitor)
	, record_handshake_timeout_(false)
	, complete_handshake_timeout_(5)
{
	//��ȡ�����ļ���Ϣ
	GET_TMAS_CONFIG_INT("global.tcp.complete-handshake-timeout", 
		                complete_handshake_timeout_);
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
void TcpConnAnalyzer::ReInit()
{
	record_handshake_timeout_ = false;

	std::memset(&fsm_info_, 0x0, sizeof(fsm_info_));

	std::memset(&conn_stat_, 0x0, sizeof(conn_stat_));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
ProcInfo TcpConnAnalyzer::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_type == MSG_PKT);
	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	if ((fsm_info_.conn_state & CONN_FAILED)
		|| (fsm_info_.conn_state & CONN_CLOSED))
	{
		return PI_RET_STOP; // ���ӹر����ٽ��б��Ĵ���
	}

	ProcessConnStat(pkt_msg);

	ProcessTcpFsm(pkt_msg);

	CheckHandshakeTimeOut(pkt_msg);

	if (pkt_msg->l4_pkt_info.l4_data_len != 0)
	{
		tcp_monitor_->TransferPktToL7(pkt_msg);
	}

	return PI_RET_STOP; // ��������ֹ
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
void TcpConnAnalyzer::ProcessTcpFsm(const PktMsgSP& pkt_msg)
{
	uint8 direction = pkt_msg->l3_pkt_info.direction;
	TMAS_ASSERT(direction < PKT_DIR_BUTT);

	HalfConnFsm& send_half_fsm = fsm_info_.half_conn_fsm[direction];
	HalfConnFsm& recv_half_fsm = fsm_info_.half_conn_fsm[(direction + 1) % 2];

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
void TcpConnAnalyzer::ProcHalfConnFsm(const PktMsgSP& pkt_msg, 
	                                  TcpFsmEvent fsm_event, 
									  HalfConnFsm& half_conn_fsm)
{
	if (half_conn_fsm.fsm_state == TFS_CLOSED) return;

	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	TcpFsmState next_state = TFS_INIT;
	if (!TcpFsm::ProcessTcpFsm(half_conn_fsm.conn_role, fsm_event, 
		                       half_conn_fsm.fsm_state, next_state))
	{
		ConnectionFailed(conn_id, CCR_ERROR);
		LOG(ERROR) << "Fail to process fsm | " << conn_id;
		return;
	}
	
	half_conn_fsm.fsm_state = next_state;

	// �����ر�����
	if (next_state == TFS_CLOSED)
	{
		ConnectionClosed(conn_id);
		DLOG(WARNING) << "Connection closed | " << conn_id;
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
void TcpConnAnalyzer::CheckHandshakeTimeOut(const PktMsgSP& pkt_msg)
{
	if (record_handshake_timeout_) return;

	uint8 direction = pkt_msg->l3_pkt_info.direction;
	TMAS_ASSERT(direction < PKT_DIR_BUTT);
	
	HalfConnFsm& send_half_fsm = fsm_info_.half_conn_fsm[direction];	
	HalfConnFsm& recv_half_fsm = fsm_info_.half_conn_fsm[(direction + 1) % 2];

	if ((send_half_fsm.fsm_state != TFS_ESTABLISHED)
		|| (recv_half_fsm.fsm_state != TFS_ESTABLISHED))
	{
		return;
	}

	conn_stat_.handshake_delay = 
		GetDurationMilliSeconds(first_pkt_time_, MicroTimeNow());

	record_handshake_timeout_ = true;

	if (conn_stat_.handshake_delay > complete_handshake_timeout_)
	{
		ProcTcpHsTimeOut(conn_stat_.handshake_delay, 
			             pkt_msg->l4_pkt_info.conn_id);
	}
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
void TcpConnAnalyzer::ProcTcpHsTimeOut(uint32 times, const ConnId& conn_id)
{
	typedef boost::date_time::local_adjustor<ptime, +8, boost::posix_time::no_dst> cn_timezone;

	//��ʽ�������ʱ��Ϣ
	std::ofstream ofs("tcp_syn.log", std::ios_base::app);
	
	// ��ȡʱ�䲿�֣� ��ת��ʱ��  
	const ptime& pt = MicroTimeNow(); 
	ptime local_pt = cn_timezone::utc_to_local(pt);

	// ��ȡʱ�䲿��  
	boost::posix_time::time_duration td = local_pt.time_of_day();  

  	ofs << boost::posix_time::to_simple_string(td) << "   " 
  		<< conn_id << "   " << times << std::endl;

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
void TcpConnAnalyzer::ConnectionFailed(const ConnId& conn_id, ConnCloseReason reason)
{
	fsm_info_.half_conn_fsm[PKT_DIR_B2S].fsm_state = TFS_CLOSED;
	fsm_info_.half_conn_fsm[PKT_DIR_S2B].fsm_state = TFS_CLOSED;

	fsm_info_.conn_state |= CONN_FAILED;

	// TODO: ��¼�����쳣�ر���Ϣ
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƴ�����
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void TcpConnAnalyzer::ConnectionClosed(const ConnId& conn_id)
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
void TcpConnAnalyzer::ProcessConnStat(const PktMsgSP& pkt_msg)
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
