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
	if (!filter_manager_.Init())
	{
		LOG(ERROR) << "Fail to init http filter manager";
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
 * 描  述: 报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 详见pkt_processor.hpp
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline ProcInfo HttpMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
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
inline void HttpMonitor<Next, Succ>::ProcessHttpRequest(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### Process http request";

	// TODO: 所有的HTTP请求都可以直接交由RunSession去处理，这样
	// 程序结构会更简单和清晰，但是，这样也会导致很多无需处理的
	// HTTP请求都会去创建RunSession，这种方式显然不够高效。

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
inline bool HttpMonitor<Next, Succ>::TryProcHttpRunSession(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### Try to process http run session";

	HttpRunSessionSP run_session;
	{
		boost::mutex::scoped_lock lock(run_session_mutex_);

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
 * 描  述: 会话完成处理
 * 参  数: [in] run_session 运行时会话
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
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
inline void HttpMonitor<Next, Succ>::SessionFailed(const HttpRunSessionSP& run_session, 
	                                               SessionFailReason reason)
{
	LOG(WARNING) << "Http session failed | " << reason;

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
inline void HttpMonitor<Next, Succ>::ConnClosedProc(const ConnId& conn_id)
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
	boost::mutex::scoped_lock lock(run_session_mutex_);

	auto iter = run_sessions_.find(conn_id);
	if (iter == run_sessions_.end()) return;

	DLOG(INFO) << "Remove one run session | " << conn_id;

	run_sessions_.erase(iter);
}

}


#endif
