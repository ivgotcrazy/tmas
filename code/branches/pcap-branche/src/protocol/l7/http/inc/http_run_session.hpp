/*#############################################################################
 * 文件名   : http_run_session.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月02日
 * 文件描述 : HttpRunSession类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RUN_SESSION
#define BROADINTER_HTTP_RUN_SESSION

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include "message.hpp"
#include "http_typedef.hpp"
#include "http_recombinder.hpp"
#include "tmas_typedef.hpp"
#include "http_recorder.hpp"

namespace BroadInter
{

using std::string;

class HttpRecorder;

/*******************************************************************************
 * 描  述: HTTP运行时session
 * 作  者: teck_zhou
 * 时  间: 2014年03月02日
 ******************************************************************************/
class HttpRunSession : public boost::enable_shared_from_this<HttpRunSession>
					 , public boost::noncopyable
{
public:
	HttpRunSession(HttpMonitorType* monitor, const ConnId& conn_id);

	//~HttpRunSession() { LOG(INFO) << "Run session state: " << session_state_; }

	// 报文处理入口
	void ProcessPacket(const PktMsgSP& pkt_msg);

	// 报文重组回调处理
	void RecombindCallback(bool result, const char* data, uint32 len);

	RecorderVec& GetRecorders() { return recorders_; }

	const RunSessionInfo& GetRunSessionInfo() const { return session_info_; }

private:
	enum RunSessionState
	{
		RSS_SESSION_INIT,				// 初始状态
		RSS_RECVING_REQUEST_HEADER,		// 正在接收请求头
		RSS_RECVING_REQUEST_DATA,		// 正在接收请求数据
		RSS_RECVED_REQUEST,				// 已经接收完整请求
		RSS_RECVING_RESPONSE_HEADER,	// 正在接收响应头
		RSS_RECVING_RESPONSE_DATA,		// 正在接收响应数据
		RSS_RECVED_RESPONSE				// 已经接收完整响应
	};

private:
	
	// 设置body传送方式(解析方式)
	void SetTransferType(const HttpHeader& http_header);

	// 状态处理函数
	void SessionInitProc(const PktMsgSP& pkt_msg);
	void RecvingRequestHeaderProc(const PktMsgSP& pkt_msg);
	void RecvingRequestDataProc(const PktMsgSP& pkt_msg);
	void RecvedRequestProc(const PktMsgSP& pkt_msg);
	void RecvingResponseHeaderProc(const PktMsgSP& pkt_msg);
	void RecvingResponseDataProc(const PktMsgSP& pkt_msg);
	void RecvedResponseProc(const PktMsgSP& pkt_msg);

	// 收到完整的HTTP请求处理
	void RecvedCompleteHttpRequest(const char* data, uint32 len);

	// 收到完整HTTP响应头处理
	void RecvedCompleteResponseHeader(const char* data, uint32 len);

	// 收到完整HTTP请求头处理
	void RecvedCompleteRequestHeader(const char* data, uint32 len);

	// 收到完整HTTP请求body处理
	void RecvedCompleteRequestData(const char* data, uint32 len);

	// 收到完整HTTP响应body处理
	void RecvedCompleteResponseData(const char* data, uint32 len);

private:
	HttpMonitorType* http_monitor_;	// 所属HttpMonitor

	RunSessionState session_state_;	// 会话状态

	RunSessionInfo session_info_;	// 会话信息

	HttpRecombinder recombinder_;	// 报文重组器

	RecorderVec recorders_;			// 所关联的记录器
};

}

#endif