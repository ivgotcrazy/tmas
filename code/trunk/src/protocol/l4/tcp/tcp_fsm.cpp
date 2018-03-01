/*#############################################################################
 * 文件名   : tcp_fsm.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月20日
 * 文件描述 : TcpFsm类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_fsm.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

TcpFsm::TcpFsmProc TcpFsm::state_proc_array_[TFS_BUTT] = \
{
	TcpFsm::ProcInitState,
	TcpFsm::ProcListenState,
	TcpFsm::ProcSynRcvState,
	TcpFsm::ProcSynSentState,
	TcpFsm::ProcEstablishedState,
	TcpFsm::ProcFinWait1State,
	TcpFsm::ProcFinWait2State,
	TcpFsm::ProcTimeWaitState,
	TcpFsm::ProcClosingState,
	TcpFsm::ProcCloseWaitState,
	TcpFsm::ProcLastAckState,
	TcpFsm::ProcClosedState
};

const char* TcpConnRoleStr[] = 
{
	"UNKONWN",
	"SERVER",
	"CLIENT"
};

const char* TcpFsmStateStr[] = 
{
	"INIT",
	"LISTEN",
	"SYN_RCV",
	"SYN_SENT",
	"ESTABLISHED",
	"FIN_WAIT_1",
	"FIN_WAIT_2",
	"TIME_WAIT",
	"CLOSING",
	"CLOSE_WAIT",
	"LAST_ACK",
	"CLOSED",
};

/*-----------------------------------------------------------------------------
 * 描  述: 状态机处理入口
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [in] curr_state 当前状态
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcessTcpFsm(TcpConnRole conn_role, TcpFsmEvent fsm_event, 
						   TcpFsmState curr_state, TcpFsmState& next_state)
{
	TMAS_ASSERT(conn_role != TCR_UNKONWN);
	TMAS_ASSERT(fsm_event != TFE_NONE);
	TMAS_ASSERT(curr_state < TFS_BUTT);

	if (curr_state >= TFS_BUTT)
	{
		LOG(ERROR) << "Invalid tcp fsm current state " << curr_state;
		return false;
	}

	DLOG(INFO) << "Process tcp fsm |"
			  << " state: " << TcpFsmStateStr[curr_state]
			  << " role: " << TcpConnRoleStr[conn_role]
			  << " event: " << (uint16)fsm_event;

	TMAS_ASSERT(state_proc_array_[curr_state]);
	
	return state_proc_array_[curr_state](conn_role, fsm_event, next_state);
}

/*-----------------------------------------------------------------------------
 * 描  述: Init状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcInitState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
	                       TcpFsmState& next_state)
{
	if (conn_role == TCR_CLIENT)
	{
		TMAS_ASSERT(fsm_event == TFE_SEND_SYN);
		next_state = TFS_SYN_SENT;
	}
	else
	{
		TMAS_ASSERT(fsm_event == TFE_RECV_SYN);
		next_state = TFS_SYN_RCV;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: Closed状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcClosedState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							 TcpFsmState& next_state)
{
	if (conn_role == TCR_CLIENT && fsm_event == TFE_SEND_SYN)
	{
		next_state = TFS_SYN_SENT;
	}
	else
	{
		DLOG(WARNING) << "Received packet on closed connection";
		next_state = TFS_CLOSED;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: Listen状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcListenState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							 TcpFsmState& next_state)
{
	LOG(ERROR) << "Invalid state";

	TMAS_ASSERT(false);
	return false;
}

/*-----------------------------------------------------------------------------
 * 描  述: SynRcv状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcSynRcvState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							 TcpFsmState& next_state)
{
	if (fsm_event & TFE_SEND_FIN)
	{
		next_state = TFS_FIN_WAIT_1;
	}
	// 客户端和服务器端都有可能走到这个状态
	else if (fsm_event == TFE_RECV_ACK)
	{
		next_state = TFS_ESTABLISHED;
	}
	else if (conn_role == TCR_SERVER && fsm_event == TFE_SEND_SYN_ACK)
	{
		next_state = TFS_SYN_RCV;
	}
	else if (conn_role == TCR_SERVER && fsm_event == TFE_SEND_ACK)
	{
		next_state = TFS_SYN_RCV;
	}
	else if (fsm_event & TFE_RECV_FIN)
	{
		next_state = TFS_CLOSE_WAIT;
	}
	else if (fsm_event & TFE_RECV_SYN)
	{
		next_state = TFS_SYN_RCV;
	}
	else
	{
		LOG(ERROR) << "### Role: " << conn_role << " Event: " << (uint16)fsm_event;
		//TMAS_ASSERT(false);
		return false;
	}
	
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: SynSent状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcSynSentState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							  TcpFsmState& next_state)
{
	if (conn_role == TCR_CLIENT && fsm_event == TFE_RECV_SYN_ACK)
	{
		next_state = TFS_ESTABLISHED;
	}
	else if (conn_role == TCR_CLIENT && fsm_event == TFE_RECV_SYN)
	{
		next_state = TFS_SYN_RCV;
	}
	else if (conn_role == TCR_CLIENT && fsm_event == TFE_SEND_ACK)
	{
		next_state = TFS_SYN_SENT;
	}
	else
	{
		LOG(ERROR) << "### Role: " << conn_role << " Event: " << (uint16)fsm_event;
		//TMAS_ASSERT(false);
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: Established状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcEstablishedState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
								  TcpFsmState& next_state)
{
	//除掉对服务器端和客户端的判断，两者都能主动终止连接
	if ((fsm_event&TFE_RECV_FIN) == TFE_RECV_FIN)
	{
		next_state = TFS_CLOSE_WAIT;
	}
	else if ((fsm_event&TFE_SEND_FIN) == TFE_SEND_FIN)
	{
		next_state = TFS_FIN_WAIT_1;
	}
	else
	{
		next_state = TFS_ESTABLISHED; // 正常的数据交互
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: FinWait1状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcFinWait1State(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							   TcpFsmState& next_state)
{
	if (fsm_event == TFE_RECV_FIN)
	{
		next_state = TFS_CLOSING;
	}
	else if (fsm_event == TFE_RECV_FIN_ACK)
	{
		next_state = TFS_TIME_WAIT; 
	}
	else if (fsm_event == TFE_RECV_ACK)
	{
		next_state = TFS_FIN_WAIT_2;
	}
	else if (fsm_event == TFE_SEND_ACK)
	{
		next_state = TFS_FIN_WAIT_1;
	}
	else
	{
		LOG(ERROR) << "### Role: " << conn_role << " Event: " << (uint16)fsm_event;
		//TMAS_ASSERT(false);
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: FinWait2状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcFinWait2State(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							   TcpFsmState& next_state)
{
	// 收到FIN后者FIN&ACK
	if (fsm_event & TFE_RECV_FIN)
	{
		next_state = TFS_TIME_WAIT;
	}
	// 有可能FIN和ACK是分开发送的
	else if (fsm_event & TFE_RECV_ACK)
	{
		next_state = TFS_FIN_WAIT_2;
	}
	else if (fsm_event & TFE_SEND_ACK)
	{
		next_state = TFS_FIN_WAIT_2;
	}
	else
	{
		LOG(ERROR) << "### Role: " << conn_role << " Event: " << (uint16)fsm_event;
		//TMAS_ASSERT(false);
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: TimeWait状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcTimeWaitState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							   TcpFsmState& next_state)
{
	next_state = TFS_TIME_WAIT;
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: Closing状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcClosingState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							  TcpFsmState& next_state)
{
	if (fsm_event == TFE_RECV_ACK)
	{
		next_state = TFS_TIME_WAIT;
	}
	else
	{
		LOG(ERROR) << "### Role: " << conn_role << " Event: " << (uint16)fsm_event;
		//TMAS_ASSERT(false);
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: CloseWait状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcCloseWaitState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
								TcpFsmState& next_state)
{
	if (fsm_event & TFE_SEND_FIN)
	{
		next_state = TFS_LAST_ACK;
	}
	else if (fsm_event == TFE_RECV_ACK)
	{
		next_state = TFS_CLOSE_WAIT;
	}
	else if (fsm_event == TFE_SEND_ACK)
	{
		// 单发Ack, 不进行跳转
		next_state = TFS_CLOSE_WAIT;
	}else
	{
		LOG(ERROR) << "### Role: " << conn_role << " Event: " << (uint16)fsm_event;
		//TMAS_ASSERT(false);  
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: LastAck状态处理
 * 参  数: [in] conn_role 连接角色
 *         [in] fsm_event 状态机事件
 *         [out] next_state 下一个状态
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年02月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcLastAckState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							  TcpFsmState& next_state)
{
	if (fsm_event == TFE_RECV_ACK)
	{
		next_state = TFS_CLOSED;
	}
	else
	{
		//在实际测试中， 发现在last_ack状态下还是有可能收到不正常的ack，故作如下处理
		next_state = TFS_LAST_ACK;
	}

	return true;
}

}