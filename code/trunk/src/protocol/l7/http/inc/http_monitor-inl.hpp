/*#############################################################################
 * 文件名   : http_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月12日
 * 文件描述 : HttpMonitor类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 * 返回值: 
 * 修  改: 
 *   时间 2014年05月07日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
HttpMonitor<Next, Succ>::HttpMonitor(const LoggerSP& logger) 
	: observer_mgr_(logger)
{

}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 消息处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月27日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 报文消息处理
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 处理结果
 * 修  改: 
 *   时间 2014年03月27日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
ProcInfo HttpMonitor<Next, Succ>::PktMsgProc(PktMsg* pkt_msg)
{
	// 当前只处理基于TCP的HTTP报文
	TMAS_ASSERT(L4_PROT(pkt_msg) == IPPROTO_TCP);

	// 如果是HTTP请求，则直接处理
	if (IsHttpRequest(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg)))
	{	
		ProcessHttpRequest(pkt_msg);
		return PI_HAS_PROCESSED;
	}

	// 再看下是否可以找到可以对应的RunSession
	if (TryProcHttpRunSession(pkt_msg))
	{
		return PI_HAS_PROCESSED;
	}

	// 不是HTTP协议报文，交由其他处理器处理
	return PI_NOT_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理HTTP请求
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月11日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 处理RunSession
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 是否处理了报文
 * 修  改:
 *   时间 2014年03月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template<class Next, class Succ>
bool HttpMonitor<Next, Succ>::TryProcHttpRunSession(const PktMsg* pkt_msg)
{
	DLOG(INFO) << "### Try to process http run session";

	HttpRunSessionSP run_session;
	{
		//boost::mutex::scoped_lock lock(run_session_mutex_);

		// RunSession不存在，则没必要再往下处理了
		auto iter = run_sessions_.find(CONN_ID(pkt_msg));
		if (iter == run_sessions_.end())
		{
			return false; // 可能不是HTTP协议报文
		}

		run_session = iter->second;
	}

	// 找到RunSession，交给RunSession处理就OK了
	run_session->ProcessPacket(pkt_msg);

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 创建新的RunSession
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: HttpRunSession
 * 修  改: 
 *   时间 2014年03月30日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 会话完成处理
 * 参  数: [in] run_session 运行时会话
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::SessionCompleted(const HttpRunSessionSP& run_session)
{
	DLOG(WARNING) << "########## Session completed";

	// 会话正常完成，记录信息
	observer_mgr_.Process(run_session->GetRunSessionInfo());

	RemoveRunSession(run_session->GetRunSessionInfo().conn_id);
}

/*-----------------------------------------------------------------------------
 * 描  述: 会话失败处理
 * 参  数: [in] run_session 运行时会话
 *         [in] reason 失败原因
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::SessionFailed(const HttpRunSessionSP& run_session, 
	                                        SessionFailReason reason)
{
	DLOG(WARNING) << "Http session failed | " << reason;

	RemoveRunSession(run_session->GetRunSessionInfo().conn_id);
}

/*-----------------------------------------------------------------------------
 * 描  述: TCP连接关闭后，对应的HTTP数据也需要删除
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
void HttpMonitor<Next, Succ>::ConnClosedProc(const ConnId& conn_id)
{
	DLOG(WARNING) << "Connection closed";

	RemoveRunSession(conn_id);
}

/*-----------------------------------------------------------------------------
 * 描  述: 删除RunSession
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
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
