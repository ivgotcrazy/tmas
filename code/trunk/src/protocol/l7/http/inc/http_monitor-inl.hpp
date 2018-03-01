/*#############################################################################
 * �ļ���   : http_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : HttpMonitor��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_HTTP_MONITOR_INL
#define BROADINTER_HTTP_MONITOR_INL

#include <string>
#include <boost/bind.hpp>
#include <glog/logging.h>

#include "http_monitor.hpp"
#include "tmas_assert.hpp"
#include "pkt_resolver.hpp"
#include "tmas_util.hpp"
#include "http_parser.hpp"
#include "http_run_session.hpp"
#include "http_recorder.hpp"

namespace BroadInter
{

#define HTTP_STATUS_200_OK	200

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
HttpMonitor<Next, Succ>::HttpMonitor(const LoggerSP& logger) 
	: observer_mgr_(logger)
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool HttpMonitor<Next, Succ>::Init()
{
	if (!observer_mgr_.Init())
	{
		LOG(ERROR) << "Fail to init http observer manager";
		return false;
	}
	
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��Ϣ����
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
ProcInfo HttpMonitor<Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	switch (msg_type)
	{
	case MSG_PKT:
		{
			PktMsg* pkt_msg = static_cast<PktMsg*>(msg_data);
			return PktMsgProc(pkt_msg);
		}
		
	case MSG_TCP_CONN_CLOSED:
		{
			ConnId* conn_id = static_cast<ConnId*>(msg_data);
			ConnClosedProc(*conn_id);
			this->PassMsgToSuccProcessor(msg_type, msg_data);
		}
		break;

	default:
		this->PassMsgToSuccProcessor(msg_type, msg_data);
		break;
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������Ϣ����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ������
 * ��  ��: 
 *   ʱ�� 2014��03��27��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
ProcInfo HttpMonitor<Next, Succ>::PktMsgProc(PktMsg* pkt_msg)
{
	// ��ǰֻ�������TCP��HTTP����
	TMAS_ASSERT(L4_PROT(pkt_msg) == IPPROTO_TCP);

	// �����HTTP������ֱ�Ӵ���
	if (IsHttpRequest(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg)))
	{	
		ProcessHttpRequest(pkt_msg);
		return PI_HAS_PROCESSED;
	}

	// �ٿ����Ƿ�����ҵ����Զ�Ӧ��RunSession
	if (TryProcHttpRunSession(pkt_msg))
	{
		return PI_HAS_PROCESSED;
	}

	// ����HTTPЭ�鱨�ģ�������������������
	return PI_NOT_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����HTTP����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::ProcessHttpRequest(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "### Process http request";

	HttpRunSessionSP run_session = NewHttpRunSession(pkt_msg);
	if (!run_session)
	{
		LOG(ERROR) << "Fail to new http run session";
		return;
	}

	run_session->ProcessPacket(pkt_msg);
}

/*------------------------------------------------------------------------------
 * ��  ��: ����RunSession
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: �Ƿ����˱���
 * ��  ��:
 *   ʱ�� 2014��03��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool HttpMonitor<Next, Succ>::TryProcHttpRunSession(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "### Try to process http run session";

	HttpRunSessionSP run_session;
	{
		//boost::mutex::scoped_lock lock(run_session_mutex_);

		// RunSession�����ڣ���û��Ҫ�����´�����
		auto iter = run_sessions_.find(CONN_ID(pkt_msg));
		if (iter == run_sessions_.end())
		{
			return false; // ���ܲ���HTTPЭ�鱨��
		}

		run_session = iter->second;
	}

	// �ҵ�RunSession������RunSession�����OK��
	run_session->ProcessPacket(pkt_msg);

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����µ�RunSession
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: HttpRunSession
 * ��  ��: 
 *   ʱ�� 2014��03��30��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
HttpRunSessionSP HttpMonitor<Next, Succ>::NewHttpRunSession(const PktMsg* pkt_msg)
{
	//boost::mutex::scoped_lock lock(run_session_mutex_);

	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	auto iter = run_sessions_.find(conn_id);
	if (iter != run_sessions_.end())
	{
		LOG(WARNING) << "Erase unfinished http run session | " 
			         << iter->second->GetRunSessionInfo().request_info.request_line.uri;
		run_sessions_.erase(iter);
	}

	HttpRunSessionMap::value_type insert_value(conn_id, 
		HttpRunSessionSP(new HttpRunSession(this, conn_id)));
	std::pair<HttpRunSessionMap::iterator, bool> insert_result;

	insert_result = run_sessions_.insert(insert_value);
	if (!insert_result.second)
	{
		LOG(ERROR) << "Fail to insert run session | " << conn_id;
		return nullptr;
	}

	return insert_result.first->second;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ự��ɴ���
 * ��  ��: [in] run_session ����ʱ�Ự
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::SessionCompleted(const HttpRunSessionSP& run_session)
{
	DLOG(WARNING) << "########## Session completed";

	// �Ự������ɣ���¼��Ϣ
	observer_mgr_.Process(run_session->GetRunSessionInfo());

	RemoveRunSession(run_session->GetRunSessionInfo().conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ựʧ�ܴ���
 * ��  ��: [in] run_session ����ʱ�Ự
 *         [in] reason ʧ��ԭ��
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::SessionFailed(const HttpRunSessionSP& run_session, 
	                                        SessionFailReason reason)
{
	DLOG(WARNING) << "Http session failed | " << reason;

	RemoveRunSession(run_session->GetRunSessionInfo().conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: TCP���ӹرպ󣬶�Ӧ��HTTP����Ҳ��Ҫɾ��
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::ConnClosedProc(const ConnId& conn_id)
{
	DLOG(WARNING) << "Connection closed";

	RemoveRunSession(conn_id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ɾ��RunSession
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::RemoveRunSession(const ConnId& conn_id)
{
	//boost::mutex::scoped_lock lock(run_session_mutex_);

	auto iter = run_sessions_.find(conn_id);
	if (iter == run_sessions_.end()) return;

	DLOG(INFO) << "Remove one run session | " << conn_id;

	run_sessions_.erase(iter);
}

}


#endif
