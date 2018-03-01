/*#############################################################################
 * 文件名   : tcp_conn_analyzer.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月09日
 * 文件描述 : TcpConnAnalyzer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: 构造函数
 * 参  数: [in] tcp_monitor TcpMonitor
 *         [in] conn_id 连接标识
 * 返回值:
 * 修  改:
 *   时间 2014年01月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
TcpConnAnalyzer<Next, Succ>::TcpConnAnalyzer(TcpMonitorType* tcp_monitor) 
	: tcp_monitor_(tcp_monitor)
	, record_handshake_timeout_(false)
	, handshake_delay_(0)
{
	//读取配置文件信息
	GET_TMAS_CONFIG_INT("global.tcp.handshake-timeout", handshake_timeout_);

	if (handshake_timeout_ > 1024000)
	{
		handshake_timeout_ = 4000;
		LOG(WARNING) << "Invalid complete-handshake-timeout " 
			         << handshake_timeout_;
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 重新初始化
 * 参  数: 
 * 返回值:
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::ReInitialize()
{
	record_handshake_timeout_ = false;

	std::memset(&fsm_info_, 0x0, sizeof(fsm_info_));

	std::memset(&conn_stat_, 0x0, sizeof(conn_stat_));
}

/*-----------------------------------------------------------------------------
 * 描  述: 消息处理总入口
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 处理结果
 * 修  改:
 *   时间 2014年03月24日
 *   作者 teck_zhou
 *   描述 创建
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
		break; // 此消息不再往下传递

	default:
		this->PassMsgToSuccProcessor(msg_type, data);
		break;
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文消息处理
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::PktMsgProc(PktMsg* pkt_msg)
{
	// 连接已经关闭或异常退出则不再处理报文
	if (fsm_info_.conn_state & (CONN_FAILED | CONN_CLOSED))
	{
		DLOG(WARNING) << "Unexpected packet | " << fsm_info_.conn_state;
		return; 
	}

	// 处理连接统计
	ProcessConnStat(pkt_msg);

	// 处理TCP状态机
	ProcessTcpFsm(pkt_msg);

	// 检查握手延时是否过长
	CheckHandshakeTimeOut(pkt_msg);

	// 如果payload不为空则需要往上继续传递
	if (pkt_msg->l4_pkt_info.l4_data_len != 0)
	{
		this->PassMsgToSuccProcessor(MSG_PKT, (void*)pkt_msg);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: TCP协议状态机处理
 * 参  数: [in] conn_info 连接信息
 *         [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年01月27日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::ProcessTcpFsm(const PktMsg* pkt_msg)
{
	TMAS_ASSERT(SEND_DIR < PKT_DIR_BUTT);

	HalfConnFsm& send_half_fsm = fsm_info_.half_conn_fsm[SEND_DIR];
	HalfConnFsm& recv_half_fsm = fsm_info_.half_conn_fsm[RECV_DIR];

	// RST标志属于异常关闭，不放在状态机内部处理
	if (RST_FLAG(pkt_msg))
	{
		DLOG(WARNING) << "Received a packet setting RST";
		ConnectionFailed(pkt_msg->l4_pkt_info.conn_id, CCR_RECV_RST);
		return;
	}
	
	// 先处理发送端状态机

	TcpFsmEvent fsm_event = TFE_NONE;
	fsm_event |= (SYN_FLAG(pkt_msg) ? TFE_SEND_SYN : TFE_NONE);
	fsm_event |= (ACK_FLAG(pkt_msg) ? TFE_SEND_ACK : TFE_NONE);
	fsm_event |= (FIN_FLAG(pkt_msg) ? TFE_SEND_FIN : TFE_NONE);

	ProcHalfConnFsm(pkt_msg, fsm_event, send_half_fsm);
	
	// 再处理接收端状态机

	fsm_event = TFE_NONE;
	fsm_event |= (SYN_FLAG(pkt_msg) ? TFE_RECV_SYN : TFE_NONE);
	fsm_event |= (ACK_FLAG(pkt_msg) ? TFE_RECV_ACK : TFE_NONE);
	fsm_event |= (FIN_FLAG(pkt_msg) ? TFE_RECV_FIN : TFE_NONE);

	ProcHalfConnFsm(pkt_msg, fsm_event, recv_half_fsm);
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理半连接FSM
 * 参  数: [in] pkt_msg 报文消息
 *         [in] fsm_event 状态机事件
 *         [in][out] half_conn_fsm 半连接FSM
 * 返回值: 
 * 修  改:
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
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

	// 正常关闭连接
	if (next_state == TFS_CLOSED)
	{
		DLOG(WARNING) << "Normally close connection | " << conn_id;

		ConnectionClosed(conn_id);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 握手超时检测
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
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

	// 报文捕获上来可能由于种种原因报文顺序已经混乱，那显然报文标记的时间
	// 先后关系也会与实际不一致，如果出现时间标记大小异常情况，我们就简单
	// 将时间标记进行调换。
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
 * 描  述: 记录tcp握手超时的连接
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改:
 *   时间 2014年03月06日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::ProcTcpHsTimeOut(const ConnId& conn_id)
{
	RecordHsTimeout(conn_id);

	tcp_monitor_->AbandonTcpConnection(conn_id);

	DLOG(WARNING) << "Abandon tcp connection for handshake timeout | " << conn_id;
}

/*-----------------------------------------------------------------------------
 * 描  述: 记录连接握手超时
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改:
 *   时间 2014年05月10日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 连接异常关闭
 * 参  数: [in] conn_info 连接信息
 *         [in] reason 连接关闭原因
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void TcpConnAnalyzer<Next, Succ>::ConnectionFailed(const ConnId& conn_id, 
	                                               ConnCloseReason reason)
{
	fsm_info_.half_conn_fsm[PKT_DIR_B2S].fsm_state = TFS_CLOSED;
	fsm_info_.half_conn_fsm[PKT_DIR_S2B].fsm_state = TFS_CLOSED;

	fsm_info_.conn_state |= CONN_FAILED;

	// TODO: 记录连接异常关闭信息
	LOG(WARNING) << "Connection failed(" << reason << ") | " << conn_id;

	RecordConnFailed(conn_id);

	DLOG(WARNING) << "Abandon failed tcp connection | " << conn_id;
	
	tcp_monitor_->AbandonTcpConnection(conn_id);
}

/*-----------------------------------------------------------------------------
 * 描  述: 记录连接异常关闭
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改:
 *   时间 2014年05月10日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 连接正常关闭处理
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 记录连接正常关闭时的握手时延信息
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改:
 *   时间 2014年05月10日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 处理连接统计
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
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