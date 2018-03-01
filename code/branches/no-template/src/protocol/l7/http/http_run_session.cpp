/*#############################################################################
 * 文件名   : http_run_session.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月02日
 * 文件描述 : HttpRunSession类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "http_run_session.hpp"
#include "tmas_assert.hpp"
#include "http_monitor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"

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
HttpRunSession::HttpRunSession(HttpMonitor* monitor, const ConnId& conn_id,
	const HttpHeader& http_header, const string& domain, const string& uri, 
	const HttpMatchType type, const string& regex_uri)
	: http_monitor_(monitor)
	, recombinder_(boost::bind(&HttpRunSession::RecombindCallback, this, _1, _2, _3))
	, session_state_(RSS_INVALID)
	, session_info_(conn_id, domain, uri, type, regex_uri)
	, match_type_(type)
{
}

/*------------------------------------------------------------------------------
 * 描  述: 收到HTTP请求处理函数
 * 参  数: [in] request_line 请求行信息
 *         [in] http_header 请求头信息
 *         [in] data 报文数据
 *         [in] len 报文长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RcvdRequestPkt(const HttpRequestLine& request_line, 
									const HttpHeader& http_header, 
									const char* data, uint32 len)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	if (session_state_ != RSS_INVALID)
	{
		TMAS_ASSERT(false);
		LOG(ERROR) << "Unexpected session state " << session_state_;
		return;
	}

	SetTransferType(http_header);

	session_state_ = RSS_RECVING_REQUEST;

	recombinder_.AppendData(data, len);
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

	if (cl_iter != http_header.end() && te_iter != http_header.end())
	{
		LOG(ERROR) << "Invalid request transfer type";
		return;
	}

	if (te_iter != http_header.end())
	{
		recombinder_.SetChunkedTransferType();
	}
	else
	{
		uint32 length = 0;
		if (cl_iter != http_header.end())
		{
			try
			{
				length = boost::lexical_cast<uint32>(cl_iter->second);
			}
			catch (...)
			{
				length = 0;
			}
		}

		recombinder_.SetClTransferType(length);
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
void HttpRunSession::RcvdResponsePkt(const HttpStatusLine& status_line, 
	                                 const HttpHeader& http_header, 
									 const char* data, uint32 len)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	if (session_state_ != RSS_RECVED_REQUEST)
	{
		TMAS_ASSERT(false);
		LOG(ERROR) << "Unexpected session state " << session_state_;
		return;
	}

	SetTransferType(http_header);

	session_state_ = RSS_RECVING_RESPONSE;

	recombinder_.AppendData(data, len);
}

/*------------------------------------------------------------------------------
 * 描  述: 收到不识别报文处理函数
 * 参  数: [in] data 报文数据
 *         [in] len 报文长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRunSession::RcvdUnrecognizedPkt(const char* data, uint32 len)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	if (session_state_ != RSS_RECVING_REQUEST 
		&& session_state_ != RSS_RECVING_RESPONSE)
	{
		LOG(ERROR) << "Unexpected session state " << session_state_;
		TMAS_ASSERT(false);
		return;
	}

	recombinder_.AppendData(data, len);
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
	if (!result)
	{
		http_monitor_->SessionFailed(shared_from_this());
		return;
	}

	if (session_state_ == RSS_RECVING_REQUEST)
	{
		ReceivedCompleteRequest(data, len);
	}
	else if (session_state_ == RSS_RECVING_RESPONSE)
	{
		ReceivedCompleteResponse(data, len);
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
void HttpRunSession::ReceivedCompleteRequest(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_REQUEST;

	data = data; // for compile warning

	session_info_.request_time = GetMicroSecond();
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
void HttpRunSession::ReceivedCompleteResponse(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_RESPONSE;

	data = data; // for compile warning

	session_info_.response_time = GetMicroSecond();
	session_info_.response_size = len;

	http_monitor_->SessionCompleted(shared_from_this());
}

}