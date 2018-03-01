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
#include "http_config_parser.hpp"
#include "http_filter_manager.hpp"

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
	// 初始化
	bool Init();

	// 报文消息处理
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// 非报文消息处理
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data);

	// RunSession通知Monitor会话已经完成
	void SessionCompleted(const HttpRunSessionSP& run_session);

	// RunSession通知Monitro会话出错了
	void SessionFailed(const HttpRunSessionSP& run_session, SessionFailReason reason);

	HttpFilterManager& GetHttpFilterManager() { return filter_manager_; }

private:

	// 处理HTTP请求
	inline void ProcessHttpRequest(const PktMsgSP& pkt_msg);

	// 处理RunSession
	inline bool TryProcHttpRunSession(const PktMsgSP& pkt_msg);

	// 创建新的RunSession
	inline HttpRunSessionSP NewHttpRunSession(const PktMsgSP& pkt_msg);
	
	// TCP连接关闭处理
	inline void ConnClosedProc(const ConnId& conn_id);

	void RemoveRunSession(const ConnId& conn_id);

private:

	typedef boost::unordered_map<ConnId, HttpRunSessionSP> HttpRunSessionMap;
	HttpRunSessionMap run_sessions_;
	boost::mutex run_session_mutex_;

	HttpFilterManager filter_manager_;
};

}

#include "http_monitor-inl.hpp"

#endif
