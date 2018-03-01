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
	if (!filter_manager_.Init())
	{
		LOG(ERROR) << "Fail to init http filter manager";
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
inline ProcInfo HttpMonitor<Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	if (msg_type == MSG_TCP_CONN_CLOSED)
	{
		const ConnId& conn_id = *(static_cast<ConnId*>(msg_data));

		ConnClosedProc(conn_id);
	}

	this->PassMsgToSuccProcessor(msg_type, msg_data);

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ���pkt_processor.hpp
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo HttpMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
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
inline void HttpMonitor<Next, Succ>::ProcessHttpRequest(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### Process http request";

	// TODO: ���е�HTTP���󶼿���ֱ�ӽ���RunSessionȥ��������
	// ����ṹ����򵥺����������ǣ�����Ҳ�ᵼ�ºܶ����账���
	// HTTP���󶼻�ȥ����RunSession�����ַ�ʽ��Ȼ������Ч��

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
inline bool HttpMonitor<Next, Succ>::TryProcHttpRunSession(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### Try to process http run session";

	HttpRunSessionSP run_session;
	{
		boost::mutex::scoped_lock lock(run_session_mutex_);

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
inline HttpRunSessionSP HttpMonitor<Next, Succ>::NewHttpRunSession(const PktMsgSP& pkt_msg)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	auto iter = run_sessions_.find(conn_id);
	if (iter != run_sessions_.end())
	{
		LOG(WARNING) << "Erase unfinished http run session | " 
			         << iter->second->GetRunSessionInfo().uri;
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
inline void HttpMonitor<Next, Succ>::SessionCompleted(const HttpRunSessionSP& run_session)
{
	DLOG(WARNING) << "########## Session completed";

	RecorderVec& recorders = run_session->GetRecorders();
	for (HttpRecorder* recorder : recorders)
	{
		recorder->RecordHttpRunSession(run_session);
	}

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
inline void HttpMonitor<Next, Succ>::SessionFailed(const HttpRunSessionSP& run_session, 
	                                               SessionFailReason reason)
{
	LOG(WARNING) << "Http session failed | " << reason;

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
inline void HttpMonitor<Next, Succ>::ConnClosedProc(const ConnId& conn_id)
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
	boost::mutex::scoped_lock lock(run_session_mutex_);

	auto iter = run_sessions_.find(conn_id);
	if (iter == run_sessions_.end()) return;

	DLOG(INFO) << "Remove one run session | " << conn_id;

	run_sessions_.erase(iter);
}

}


#endif
