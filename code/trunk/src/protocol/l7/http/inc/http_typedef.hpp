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

	ConnId conn_id;				// ��������
    PktDirection request_direction;  // HTTP���������Ǵ��IP��ַ����С��IP��ַ

	string request;				// ����ͷ�ֶε���ʱʹ��
	string response;			// ��Ӧͷ�ֶε���ʱʹ��

	HttpRequest request_info;	// ������Ϣ

	string request_header;		// ԭʼ����ͷ
	string response_header;		// ԭʼ��Ӧͷ

	uint32 request_size;		// �����С
	uint32 response_size;		// ��Ӧ��С
	uint32 status_code;			// ״̬��

	uint64 request_end;			// �����������ʱ��
	uint64 response_begin;		// ������Ӧ��ʼʱ��
	uint64 response_end;		// ������Ӧ����ʱ��
};

typedef RunSessionInfo HttpRecordInfo;

class HttpFilter;
typedef boost::shared_ptr<HttpFilter> HttpFilterSP;

class HttpRecorder;
typedef boost::shared_ptr<HttpRecorder> HttpRecorderSP;

}

#endif
