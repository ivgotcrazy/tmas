/*#############################################################################
 * 文件名   : http_typedef.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月02日
 * 文件描述 : HTTP相关类型声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_TYPEDEF
#define BROADINTER_HTTP_TYPEDEF

#include <boost/unordered_map.hpp>

#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

using std::string;

enum HttpPktType
{
	HTTP_PKT_UNKNOWN,
	HTTP_PKT_REQUEST,
	HTTP_PKT_RESPONSE
};

// HTTP消息头容器
typedef boost::unordered_map<std::string, std::string> HttpHeader;

// HTTP请求行
struct HttpRequestLine
{
	std::string method;
	std::string uri;
	std::string version;
};

// HTTP响应状态行
struct HttpStatusLine
{
	std::string version;
	uint16 status_code;
};

// HTTP消息体
struct HttpBody
{
	char* start;
	uint32 len;
};

// HTTP请求信息
struct HttpRequest
{
	HttpRequestLine request_line;
	HttpHeader header;
	HttpBody body;
};

// HTTP响应信息
struct HttpResponse
{
	HttpStatusLine status_line;
	HttpHeader header;
	HttpBody body;
};

enum HttpTransferType
{
	HTT_UNKNOWN,
	HTT_CONTENT_LENGTH,
	HTT_CHUNKED
};

enum SessionFailReason
{
	SFR_UNEXPECTED_PKT,
	SFR_PARSE_REQUEST_ERR,
	SFR_NOT_INTEREST_REQUEST,
	SFR_PARSE_RESPONSE_ERR,
	SFR_RECOMBIND_ERR
};

enum SessionStatType
{
	SST_NONE,
	SST_URI,
	SST_DOMAIN
};

struct RunSessionInfo
{
	RunSessionInfo(const ConnId& conn)
		: conn_id(conn)
		, request_size(0)
		, response_size(0)
		, request_end(0)
		, response_begin(0)
		, response_end(0)
	{}

	ConnId conn_id;				// 所属连接
    PktDirection request_direction;  // HTTP请求发起者是大的IP地址还是小的IP地址

	string request;				// 请求头分段到达时使用
	string response;			// 响应头分段到达时使用

	HttpRequest request_info;	// 请求信息

	string request_header;		// 原始请求头
	string response_header;		// 原始响应头

	uint32 request_size;		// 请求大小
	uint32 response_size;		// 响应大小
	uint32 status_code;			// 状态码

	uint64 request_end;			// 接收请求结束时间
	uint64 response_begin;		// 接收响应开始时间
	uint64 response_end;		// 接收响应结束时间
};

typedef RunSessionInfo HttpRecordInfo;

class HttpFilter;
typedef boost::shared_ptr<HttpFilter> HttpFilterSP;

class HttpRecorder;
typedef boost::shared_ptr<HttpRecorder> HttpRecorderSP;

}

#endif
