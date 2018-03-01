/*#############################################################################
 * �ļ���   : http_typedef.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��02��
 * �ļ����� : HTTP�����������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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

// HTTP��Ϣͷ����
typedef boost::unordered_map<std::string, std::string> HttpHeader;

// HTTP������
struct HttpRequestLine
{
	std::string method;
	std::string uri;
	std::string version;
};

// HTTP��Ӧ״̬��
struct HttpStatusLine
{
	std::string version;
	uint16 status_code;
};

// HTTP��Ϣ��
struct HttpBody
{
	char* start;
	uint32 len;
};

// HTTP������Ϣ
struct HttpRequest
{
	HttpRequestLine request_line;
	HttpHeader header;
	HttpBody body;
};

// HTTP��Ӧ��Ϣ
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

//��Դƥ������
enum HttpMatchType
{
	DOMAIN_MATCH_TYPE,
	URI_MATCH_TYPE,
};
}

#endif