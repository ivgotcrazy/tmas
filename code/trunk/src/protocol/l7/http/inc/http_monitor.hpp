/*#############################################################################
 * 文件名   : http_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月12日
 * 文件描述 : HttpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: HTTP数据监听
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
template<class Next, class Succ>
class HttpMonitor : public PktProcessor<HttpMonitor<Next, Succ>, Next, Succ>
{
public:
	HttpMonitor(const LoggerSP& logger);

	// 初始化
	bool Init();

	// 消息处理
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data);

	// RunSession通知Monitor会话已经完成
	void SessionCompleted(const HttpRunSessionSP& run_session);

	// RunSession通知Monitro会话出错了
	void SessionFailed(const HttpRunSessionSP& run_session, SessionFailReason reason);

	HttpObserverManager& GetHttpObserverManager() { return observer_mgr_; }

private:
	ProcInfo PktMsgProc(PktMsg* pkt_msg);

	// 处理HTTP请求
	void ProcessHttpRequest(const PktMsg* pkt_msg);

	// 处理RunSession
	bool TryProcHttpRunSession(const PktMsg* pkt_msg);

	// 创建新的RunSession
	HttpRunSessionSP NewHttpRunSession(const PktMsg* pkt_msg);
	
	// TCP连接关闭处理
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
