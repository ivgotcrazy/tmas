/*#############################################################################
 * �ļ���   : tcp_fsm.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��20��
 * �ļ����� : TcpFsm��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ״̬���������
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [in] curr_state ��ǰ״̬
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: Init״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: Closed״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: Listen״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcListenState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							 TcpFsmState& next_state)
{
	LOG(ERROR) << "Invalid state";

	TMAS_ASSERT(false);
	return false;
}

/*-----------------------------------------------------------------------------
 * ��  ��: SynRcv״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcSynRcvState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							 TcpFsmState& next_state)
{
	if (fsm_event & TFE_SEND_FIN)
	{
		next_state = TFS_FIN_WAIT_1;
	}
	// �ͻ��˺ͷ������˶��п����ߵ����״̬
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
 * ��  ��: SynSent״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: Established״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcEstablishedState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
								  TcpFsmState& next_state)
{
	//�����Է������˺Ϳͻ��˵��жϣ����߶���������ֹ����
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
		next_state = TFS_ESTABLISHED; // ���������ݽ���
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: FinWait1״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: FinWait2״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcFinWait2State(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							   TcpFsmState& next_state)
{
	// �յ�FIN����FIN&ACK
	if (fsm_event & TFE_RECV_FIN)
	{
		next_state = TFS_TIME_WAIT;
	}
	// �п���FIN��ACK�Ƿֿ����͵�
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
 * ��  ��: TimeWait״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool TcpFsm::ProcTimeWaitState(TcpConnRole conn_role, TcpFsmEvent fsm_event,
							   TcpFsmState& next_state)
{
	next_state = TFS_TIME_WAIT;
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: Closing״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: CloseWait״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
		// ����Ack, ��������ת
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
 * ��  ��: LastAck״̬����
 * ��  ��: [in] conn_role ���ӽ�ɫ
 *         [in] fsm_event ״̬���¼�
 *         [out] next_state ��һ��״̬
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��02��20��
 *   ���� teck_zhou
 *   ���� ����
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
		//��ʵ�ʲ����У� ������last_ack״̬�»����п����յ���������ack���������´���
		next_state = TFS_LAST_ACK;
	}

	return true;
}

}