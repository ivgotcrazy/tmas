/*#############################################################################
 * 文件名   : http_parser.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月12日
 * 文件描述 : HttpParser类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_PARSER
#define BROADINTER_HTTP_PARSER 

#include "tmas_typedef.hpp"
#include "http_typedef.hpp"

namespace BroadInter
{

//==============================================================================

namespace HP // http parser
{

bool ParseHttpRequest(const char* data, 
		uint32 len, 
		HttpRequest& request);

bool ParseHttpResponse(const char* data, 
		uint32 len, 
		HttpResponse& response);

bool ParseHttpRequestLine(const char* data, 
		uint32 len, 
		HttpRequestLine& request_line, 
		uint32& parse_len);

bool ParseHttpStatusLine(const char* data, 
		uint32 len, 
		HttpStatusLine& status_line, 
		uint32& parse_len);

bool ParseHttpHeader(const char* data, 
		uint32 len, 
		HttpHeader& http_header, 
		uint32& parse_len);
}

//==============================================================================

namespace HPN // http parser new
{

// 请求行解析状态
enum RequestLineParseState
{
	RLPS_INIT,
	RLPS_METHOD,
	RLPS_METHOD_POST_SPACE,
	RLPS_URI,
	RLPS_URI_POST_SPACE,
	RLPS_VERSION,
	RLPS_VERSION_POST_LF,
	RLPS_STOP
};

// 状态行解析状态
enum StatusLineParseState
{
	SLPS_INIT,
	SLPS_VERSION,
	SLPS_VERSION_POST_SPACE,
	SLPS_STATUS_CODE,
	SLPS_STATUS_CODE_POST_SPACE,
	SLPS_REASON,
	SLPS_REASON_POST_LF,
	SLPS_STOP
};

// HTTP头解析状态
enum HttpHeaderParseState
{
	HHPS_INIT,
	HHPS_KEY,
	HHPS_COLON,
	HHPS_VALUE,
	HHPS_VALUE_POST_LF,
	HHPS_STOP
};

// Body解析状态
enum HttpBodyParseState
{
	HBPS_INIT,
	HBPS_BODY_SEPERATOR,
	HBPS_DATA
};

bool GetHttpHeader(const char* data, 
		uint32 len, 
		std::string& header);

bool ParseHttpRequest(const char* data, 
		uint32 len, 
		HttpRequest& request);

bool ParseHttpResponse(const char* data, 
		uint32 len, 
		HttpResponse& response);

bool ParseHttpRequestLine(const char* data, 
		uint32 len, 
		RequestLineParseState& curr_state, 
		HttpRequestLine& request_line,
		uint32& parse_len);

bool ParseHttpStatusLine(const char* data, 
		uint32 len, 
		StatusLineParseState& curr_state, 
		HttpStatusLine& status_line, 
		uint32& parse_len);

bool ParseHttpHeader(const char* data, 
		uint32 len, 
		HttpHeaderParseState& curr_state, 
		HttpHeader& http_header, 
		uint32& parse_len);

bool ParseHttpBody(const char* data, 
		uint32 len, 
		HttpBodyParseState& curr_state, 
		HttpBody& http_body);

}

bool IsHttpRequest(const char* data, uint32 len);

bool IsHttpResponse(const char* data, uint32 len);

bool HasCompleteHttpHeader(const char* data, uint32 len);

bool IsCompleteHttpRequest(const char* data, uint32 len);


}

#endif