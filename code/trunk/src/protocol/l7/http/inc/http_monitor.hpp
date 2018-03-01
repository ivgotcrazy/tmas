/*#############################################################################
 * �ļ���   : http_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : HttpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_MONITOR
#define BROADINTER_HTTP_MONITOR

#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "timer.hpp"
#include "http_typedef.hpp"
#include "http_observer_manager.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: HTTP���ݼ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
template<class Next, class Succ>
class HttpMonitor : public PktProcessor<HttpMonitor<Next, Succ>, Next, Succ>
{
public:
	HttpMonitor(const LoggerSP& logger);

	// ��ʼ��
	bool Init();

	// ��Ϣ����
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data);

	// RunSession֪ͨMonitor�Ự�Ѿ����
	void SessionCompleted(const HttpRunSessionSP& run_session);

	// RunSession֪ͨMonitro�Ự������
	void SessionFailed(const HttpRunSessionSP& run_session, SessionFailReason reason);

	HttpObserverManager& GetHttpObserverManager() { return observer_mgr_; }

private:
	ProcInfo PktMsgProc(PktMsg* pkt_msg);

	// ����HTTP����
	void ProcessHttpRequest(const PktMsg* pkt_msg);

	// ����RunSession
	bool TryProcHttpRunSession(const PktMsg* pkt_msg);

	// �����µ�RunSession
	HttpRunSessionSP NewHttpRunSession(const PktMsg* pkt_msg);
	
	// TCP���ӹرմ���
	void ConnClosedProc(const ConnId& conn_id);

	void RemoveRunSession(const ConnId& conn_id);

private:

	typedef boost::unordered_map<ConnId, HttpRunSessionSP> HttpRunSessionMap;
	HttpRunSessionMap run_sessions_;
	
	//boost::mutex run_session_mutex_;

	HttpObserverManager observer_mgr_;
};

}

#include "http_monitor-inl.hpp"

#endif
