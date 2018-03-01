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

namespace BroadInter
{

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

//资源匹配类型
enum HttpMatchType
{
	DOMAIN_MATCH_TYPE,
	URI_MATCH_TYPE,
};
}

#endif