/*#############################################################################
 * 文件名   : tcp_fsm.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月20日
 * 文件描述 : TcpFsm类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_TCP_FSM
#define BROADINTER_TMAS_TCP_FSM

#include "tmas_typedef.hpp"
#include "tcp_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: TCP状态机处理静态类
 * 作  者: teck_zhou
 * 时  间: 2014年02月20日
 ******************************************************************************/
class TcpFsm
{
public:
	static bool ProcessTcpFsm(TcpConnRole conn_role, 
		                      TcpFsmEvent fsm_event, 
						      TcpFsmState curr_state, 
						      TcpFsmState& next_state);
private:
	static bool ProcInitState(TcpConnRole conn_role, 
		                      TcpFsmEvent fsm_event,
							  TcpFsmState& next_state);
	static bool ProcClosedState(TcpConnRole conn_role, 
							    TcpFsmEvent fsm_event,
							    TcpFsmState& next_state);
	static bool ProcListenState(TcpConnRole conn_role, 
		                        TcpFsmEvent fsm_event,
		                        TcpFsmState& next_state);
	static bool ProcSynRcvState(TcpConnRole conn_role, 
								TcpFsmEvent fsm_event,
								TcpFsmState& next_state);
	static bool ProcSynSentState(TcpConnRole conn_role, 
								 TcpFsmEvent fsm_event,
								 TcpFsmState& next_state);
	static bool ProcEstablishedState(TcpConnRole conn_role, 
							         TcpFsmEvent fsm_event,
									 TcpFsmState& next_state);
	static bool ProcFinWait1State(TcpConnRole conn_role, 
								  TcpFsmEvent fsm_event,
								  TcpFsmState& next_state);
	static bool ProcFinWait2State(TcpConnRole conn_role, 
								  TcpFsmEvent fsm_event,
								  TcpFsmState& next_state);
	static bool ProcTimeWaitState(TcpConnRole conn_role, 
								  TcpFsmEvent fsm_event,
								  TcpFsmState& next_state);
	static bool ProcClosingState(TcpConnRole conn_role, 
								 TcpFsmEvent fsm_event,
								 TcpFsmState& next_state);
	static bool ProcCloseWaitState(TcpConnRole conn_role, 
								   TcpFsmEvent fsm_event,
								   TcpFsmState& next_state);
	static bool ProcLastAckState(TcpConnRole conn_role, 
								 TcpFsmEvent fsm_event,
								 TcpFsmState& next_state);

private:
	typedef boost::function<bool(TcpConnRole, TcpFsmEvent,TcpFsmState&)> TcpFsmProc;
	static TcpFsmProc state_proc_array_[TFS_BUTT];
};

}

#endif
