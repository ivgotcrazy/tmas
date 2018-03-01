/*#############################################################################
 * 文件名   : http_run_session.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月02日
 * 文件描述 : HttpRunSession类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "http_run_session.hpp"
#include "tmas_assert.hpp"
#include "http_monitor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "message.hpp"
#include "http_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: 略
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpRunSession::HttpRunSession(HttpMonitorType* monitor, const ConnId& conn_id)
	: http_monitor_(monitor)
	, session_state_(RSS_SESSION_INIT)
	, session_info_(conn_id)
	, recombinder_(this, false)
{
}

/*------------------------------------------------------------------------------
 * 描  述: 基于状态的报文处理
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::ProcessPacket(const PktMsgSP& pkt_msg)
{
	switch (session_state_)
	{
	case RSS_SESSION_INIT:
		SessionInitProc(pkt_msg);
		break;

	case RSS_RECVING_REQUEST_HEADER:
		RecvingRequestHeaderProc(pkt_msg);
		break;

	case RSS_RECVING_REQUEST_DATA:
		RecvingRequestDataProc(pkt_msg);
		break;

	case RSS_RECVED_REQUEST:
		RecvedRequestProc(pkt_msg);
		break;

	case RSS_RECVING_RESPONSE_HEADER:
		RecvingResponseHeaderProc(pkt_msg);
		break;

	case RSS_RECVING_RESPONSE_DATA:
		RecvingResponseDataProc(pkt_msg);
		break;

	case RSS_RECVED_RESPONSE:
		RecvedResponseProc(pkt_msg);
		break;

	default:
		TMAS_ASSERT(false);
		LOG(ERROR) << "Unexpected state " << session_state_;
		return;
	}
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_SESSION_INIT状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::SessionInitProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::SessionInitProc";

	const char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	if (IsCompleteHttpRequest(data, len))
	{
		RecvedCompleteHttpRequest(data, len);
	}
	else if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteRequestHeader(data, len);
	}
	else
	{
		session_info_.request.assign(data, len);
		session_state_ = RSS_RECVING_REQUEST_HEADER;
	}
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_RECVING_REQUEST_HEADER状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingRequestHeaderProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingRequestHeaderProc";

	TMAS_ASSERT(!session_info_.request.empty());

	session_info_.request.append(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));

	const char* data = session_info_.request.c_str();
	uint32 len = session_info_.request.length();

	if (IsCompleteHttpRequest(data, len))
	{
		RecvedCompleteHttpRequest(data, len);
	}
	else if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteRequestHeader(data, len);
	}
	else
	{
		; // 继续接收
	}
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_RECVING_REQUEST_DATA状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingRequestDataProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingRequestHeaderProc";

	recombinder_.AppendData(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_RECVED_REQUEST状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedRequestProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvedRequestProc";

	const char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	if (!IsHttpResponse(data, len))
	{
		DLOG(WARNING) << "Should come http response";
		http_monitor_->SessionFailed(shared_from_this(), SFR_UNEXPECTED_PKT);
		return;
	}

	// 开始接收HTTP响应
	session_info_.response_begin = GetMicroSecond();

	if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteResponseHeader(data, len);
	}
	else
	{
		session_info_.response.assign(data, len);
		session_state_ = RSS_RECVING_RESPONSE_HEADER;
	}
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_RECVING_RESPONSE_HEADER状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingResponseHeaderProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingResponseHeaderProc";

	TMAS_ASSERT(!session_info_.response.empty());

	session_info_.response.append(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));

	const char* data = session_info_.response.c_str();
	uint32 len = session_info_.response.length();

	if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteResponseHeader(data, len);
	}
	else
	{
		; // 继续接收
	}
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_RECVING_RESPONSE_DATA状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingResponseDataProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingResponseDataProc";

	recombinder_.AppendData(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));
}

/*------------------------------------------------------------------------------
 * 描  述: RSS_RECVED_RESPONSE状态处理
 * 参  数: [in] pkt_msg
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedResponseProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvedResponseProc";

	TMAS_ASSERT(false);
}

/*------------------------------------------------------------------------------
 * 描  述: 接收到完整HTTP请求处理
 * 参  数: [in] data 请求数据
 *         [in] len 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteHttpRequest(const char* data, uint32 len)
{
	TMAS_ASSERT((session_state_ == RSS_SESSION_INIT)
		|| (session_state_ == RSS_RECVING_REQUEST_HEADER));

	HttpRequest http_request;
	if (!HPN::ParseHttpRequest(data, len, http_request))
	{
		DLOG(WARNING) << "Fail to parse http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_PARSE_REQUEST_ERR);
		return;
	}

	session_info_.uri = http_request.request_line.uri;

	HttpFilterManager& filter_manager = http_monitor_->GetHttpFilterManager();
	filter_manager.GetMatchedRecorders(http_request, session_info_.conn_id.smaller_ip,
										session_info_.conn_id.bigger_ip, recorders_);
	if (recorders_.empty())
	{
		DLOG(WARNING) << "Not interest http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_NOT_INTEREST_REQUEST);
		return;
	}

	session_info_.request.clear();

	session_info_.request_end = GetMicroSecond();

	session_info_.request_size = len;

	session_state_ = RSS_RECVED_REQUEST;
}

/*------------------------------------------------------------------------------
 * 描  述: 接收到完整HTTP请求头处理
 * 参  数: [in] data 请求数据
 *         [in] len 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteRequestHeader(const char* data, uint32 len)
{
	TMAS_ASSERT((session_state_ == RSS_SESSION_INIT)
		|| (session_state_ == RSS_RECVING_REQUEST_HEADER));

	HttpRequest http_request;
	if (!HPN::ParseHttpRequest(data, len, http_request))
	{
		DLOG(ERROR) << "Fail to parse http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_PARSE_REQUEST_ERR);
		return;
	}

	session_info_.uri = http_request.request_line.uri;

	HttpFilterManager& filter_manager = http_monitor_->GetHttpFilterManager();
	filter_manager.GetMatchedRecorders(http_request, session_info_.conn_id.smaller_ip,
										session_info_.conn_id.bigger_ip, recorders_);
	if (recorders_.empty())
	{
		DLOG(WARNING) << "Not interest http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_NOT_INTEREST_REQUEST);
		return;
	}

	session_info_.request.clear();

	session_state_ = RSS_RECVING_REQUEST_DATA;

	SetTransferType(http_request.header);

	if (http_request.body.len > 0)
	{
		recombinder_.AppendData(http_request.body.start, http_request.body.len);
	}
}

/*------------------------------------------------------------------------------
 * 描  述: 收到HTTP响应处理函数
 * 参  数: [in] status_line 响应行信息
 *         [in] http_header 请求头信息
 *         [in] data 报文数据
 *         [in] len 报文长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteResponseHeader(const char* data, uint32 len)
{
	TMAS_ASSERT((session_state_ == RSS_RECVING_RESPONSE_HEADER)
		|| (session_state_ == RSS_RECVED_REQUEST));

	HttpResponse http_response;
	if (!HPN::ParseHttpResponse(data, len, http_response))
	{
		DLOG(ERROR) << "Fail to parse http response";
		http_monitor_->SessionFailed(shared_from_this(), SFR_PARSE_RESPONSE_ERR);
		return;
	}

	SetTransferType(http_response.header);

	session_state_ = RSS_RECVING_RESPONSE_DATA;

	// 不管数据长度是否为0，都需要传递到recombinder，进而来确定响应是否完成
	recombinder_.AppendData(http_response.body.start, http_response.body.len);
}

/*------------------------------------------------------------------------------
 * 描  述: 收到HTTP完整报文处理函数
 * 参  数: [in] result 成功/失败
 *         [in] data 数据
 *         [in] len 长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecombindCallback(bool result, const char* data, uint32 len)
{
	// 接收HTTP响应结束
	session_info_.response_end = GetMicroSecond();

	if (!result)
	{
		http_monitor_->SessionFailed(shared_from_this(), SFR_RECOMBIND_ERR);
		return;
	}

	if (session_state_ == RSS_RECVING_REQUEST_DATA)
	{
		RecvedCompleteRequestData(data, len);
	}
	else if (session_state_ == RSS_RECVING_RESPONSE_DATA)
	{
		RecvedCompleteResponseData(data, len);
	}
	else
	{
		TMAS_ASSERT(false);
		LOG(ERROR) << "Invalid session state " << session_state_;
	}
}

/*------------------------------------------------------------------------------
 * 描  述: 收到HTTP请求处理
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteRequestData(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_REQUEST;

	data = data; // for compile warning

	session_info_.request_end = GetMicroSecond();
	session_info_.request_size = len;
}

/*------------------------------------------------------------------------------
 * 描  述: 收到HTTP响应处理
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteResponseData(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_RESPONSE;

	data = data; // for compile warning

	session_info_.response_size = len;

	http_monitor_->SessionCompleted(shared_from_this());
}

/*------------------------------------------------------------------------------
 * 描  述: 收到HTTP响应处理函数
 * 参  数: [in] http_header HTTP协议头
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::SetTransferType(const HttpHeader& http_header)
{
	auto cl_iter = http_header.find("content-length");
	auto te_iter = http_header.find("transfer-encoding");

	TMAS_ASSERT(!(cl_iter != http_header.end() && te_iter != http_header.end()));

	if (cl_iter == http_header.end() && te_iter == http_header.end())
	{
		recombinder_.SetUnknownTransferType();
	}
	else if (te_iter != http_header.end())
	{
		recombinder_.SetChunkedTransferType();
	}
	else
	{
		uint32 length = 0;
		try
		{
			length = boost::lexical_cast<uint32>(cl_iter->second);
		}
		catch (...)
		{
			LOG(ERROR) << "Invalid content-length : " << cl_iter->second;
		}

		recombinder_.SetClTransferType(length);
	}
}

}