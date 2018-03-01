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

namespace BroadInter
{

using std::string;

class HttpMonitor;

/*******************************************************************************
 * 描  述: HTTP运行时session
 * 作  者: teck_zhou
 * 时  间: 2014年03月02日
 ******************************************************************************/
class HttpRunSession : public boost::enable_shared_from_this<HttpRunSession>
					 , public boost::noncopyable
{
public:
	struct RunSessionInfo
	{
		RunSessionInfo(
			const ConnId& conn, 
			const std::string& d, 
			const std::string& u,
			const HttpMatchType type,
			const std::string& regex_uri)
			: conn_id(conn)
			, domain(d)
			, uri(u)
			, request_size(0)
			, response_size(0)
			, match_type(type)
			, match_uri(regex_uri)
		{}

		ConnId conn_id; // 所属连接
		string domain;	// URI所属域名
		string uri;		// URI

		uint32 request_size;
		uint32 response_size;

		uint64 request_time;
		uint64 response_time;

		HttpMatchType match_type;
		string match_uri;
	};

	enum RunSessionState
	{
		RSS_INVALID,

		RSS_RECVING_REQUEST,	// 正在接收请求
		RSS_RECVED_REQUEST,		// 已经接收请求
		RSS_RECVING_RESPONSE,	// 正在接收响应
		RSS_RECVED_RESPONSE		// 已经接收响应
	};

public:
	HttpRunSession(HttpMonitor* monitor, 
		const ConnId& conn_id, const HttpHeader& http_header, const string& domain, 
		const string& uri, const HttpMatchType type, const string& regex_uri);

	// 收到HTTP请求报文处理
	void RcvdRequestPkt(const HttpRequestLine& request_line, 
		const HttpHeader& http_header, const char* data, uint32 len);

	// 收到HTTP响应报文处理
	void RcvdResponsePkt(const HttpStatusLine& status_line,
		const HttpHeader& http_header, const char* data, uint32 len);

	// 收到不能识别报文处理
	void RcvdUnrecognizedPkt(const char* data, uint32 len);

	const RunSessionInfo& GetRunSessionInfo() const { return session_info_; }

	RunSessionState GetRunSessionState() const { return session_state_; }

private:
	// 报文重组回调处理
	void RecombindCallback(bool result, const char* data, uint32 len);

	// 收到完整HTTP请求报文处理
	void ReceivedCompleteRequest(const char* data, uint32 len);

	// 收到完整HTTP响应报文处理
	void ReceivedCompleteResponse(const char* data, uint32 len);

	// 设置body传送方式(解析方式)
	void SetTransferType(const HttpHeader& http_header);

private:
	HttpMonitor* http_monitor_;

	HttpRecombinder recombinder_;

	RunSessionState session_state_;

	RunSessionInfo session_info_;

	boost::mutex run_session_mutex_;

	HttpMatchType match_type_;
};

}

#endif