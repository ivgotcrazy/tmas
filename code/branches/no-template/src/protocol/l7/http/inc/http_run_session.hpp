/*#############################################################################
 * �ļ���   : http_run_session.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��02��
 * �ļ����� : HttpRunSession������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: HTTP����ʱsession
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��02��
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

		ConnId conn_id; // ��������
		string domain;	// URI��������
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

		RSS_RECVING_REQUEST,	// ���ڽ�������
		RSS_RECVED_REQUEST,		// �Ѿ���������
		RSS_RECVING_RESPONSE,	// ���ڽ�����Ӧ
		RSS_RECVED_RESPONSE		// �Ѿ�������Ӧ
	};

public:
	HttpRunSession(HttpMonitor* monitor, 
		const ConnId& conn_id, const HttpHeader& http_header, const string& domain, 
		const string& uri, const HttpMatchType type, const string& regex_uri);

	// �յ�HTTP�����Ĵ���
	void RcvdRequestPkt(const HttpRequestLine& request_line, 
		const HttpHeader& http_header, const char* data, uint32 len);

	// �յ�HTTP��Ӧ���Ĵ���
	void RcvdResponsePkt(const HttpStatusLine& status_line,
		const HttpHeader& http_header, const char* data, uint32 len);

	// �յ�����ʶ���Ĵ���
	void RcvdUnrecognizedPkt(const char* data, uint32 len);

	const RunSessionInfo& GetRunSessionInfo() const { return session_info_; }

	RunSessionState GetRunSessionState() const { return session_state_; }

private:
	// ��������ص�����
	void RecombindCallback(bool result, const char* data, uint32 len);

	// �յ�����HTTP�����Ĵ���
	void ReceivedCompleteRequest(const char* data, uint32 len);

	// �յ�����HTTP��Ӧ���Ĵ���
	void ReceivedCompleteResponse(const char* data, uint32 len);

	// ����body���ͷ�ʽ(������ʽ)
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