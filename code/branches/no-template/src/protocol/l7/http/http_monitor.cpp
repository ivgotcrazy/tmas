/*#############################################################################
 * �ļ���   : http_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : HttpMonitor��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <string>
#include <boost/bind.hpp>
#include <glog/logging.h>

#include "http_monitor.hpp"

#include "tmas_assert.hpp"
#include "pkt_resolver.hpp"
#include "tmas_util.hpp"
#include "http_parser.hpp"
#include "http_run_session.hpp"

namespace BroadInter
{

#define HTTP_STATUS_200_OK	200

/*------------------------------------------------------------------------------
 * ��  ��: HttpMonitor��Ĺ��캯��
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014.02.24
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
HttpMonitor::HttpMonitor()
{
}

/*------------------------------------------------------------------------------
 * ��  ��: HttpMonitor�����������
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014.02.24
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
HttpMonitor::~HttpMonitor()
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/

bool HttpMonitor::Init()
{
	if (!LoadConfiguration())
	{
		LOG(ERROR) << "Fail to load configuration";
		return false;
	}

	speed_test_timer_.reset(
		new FreeTimer(boost::bind(&HttpMonitor::SpeedTestProc, this), 5));
	speed_test_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������ļ�
 * ��  ��: 
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::LoadConfiguration()
{
    bool ok = false;
	HttpConfigParser parser;
    ok = parser.Parse("../config/http.conf", host_site_map_, uri_site_map_);
	
    return ok;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ���pkt_processor.hpp
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
ProcInfo HttpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	if (msg_type == MSG_PKT)
	{
		PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

		if (L4_PROT(pkt_msg) != IPPROTO_TCP)
		{
			return PI_CHAIN_CONTINUE;
		}
		
		if (IsHttpRequest(pkt_msg))
		{	
			ProcessHttpRequest(pkt_msg);		
			return PI_RET_STOP;
		}

		if (TryProcHttpRunSession(pkt_msg))
		{
			return PI_RET_STOP;
		}

		return PI_CHAIN_CONTINUE;
	}
	else if (msg_type == MSG_TCP_CONN_CLOSED)
	{
		const ConnId& conn_id = *(boost::static_pointer_cast<ConnId>(msg_data));

		ConnClosedProc(conn_id);

		return PI_CHAIN_CONTINUE;
	}
	else
	{
		LOG(ERROR) << "Unknown message type";
		TMAS_ASSERT(false);
	}

	return PI_RET_STOP;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����HTTP����
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HttpMonitor::ProcessHttpRequest(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Process http request";

	if (!IfRequestHasCompleteHeader(pkt_msg))
	{
		DLOG(WARNING) << "Incomplete http header";
		return;
	}

	HttpRequestLine request_line;
	HttpHeader http_header;
	uint32 parse_len = 0;
	if (!ParseRequestLineAndHeader(pkt_msg, request_line, 
		                           http_header, parse_len))
	{
		LOG(ERROR) << "Fail to parse http request";
		return;
	}

	HttpMatchType type;
	std::string regex_uri;
	if (!IsInterestHttpRequest(http_header, request_line, type, regex_uri))
	{
		DLOG(WARNING) << "Is not interesting http request";
		return ;
	}
	
	ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;
	HttpRunSessionSP run_session = 
		NewHttpRunSession(conn_id, request_line, http_header, type, regex_uri);
	if (!run_session)
	{
		LOG(ERROR) << "Fail to new http run session";
		return;
	}
	
	run_session->RcvdRequestPkt(request_line, http_header, 
		L7_DATA(pkt_msg) + parse_len, L4_DATA_LEN(pkt_msg) - parse_len);
}

/*------------------------------------------------------------------------------
 * ��  ��: ����RunSession
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: �Ƿ����˱���
 * ��  ��:
 *   ʱ�� 2014��03��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpMonitor::TryProcHttpRunSession(const PktMsgSP& pkt_msg)
{
	DLOG(WARNING) << "Try to process http run session";

	boost::mutex::scoped_lock lock(run_session_mutex_);

	//RunSession�����ڣ���û��Ҫ�����´����ˣ���ʹ��HttpResponse
	auto iter = run_sessions_.find(CONN_ID(pkt_msg));
	if (iter == run_sessions_.end()) return false;

	HttpRunSessionSP& run_session = iter->second;

	bool is_response = IsHttpResponse(pkt_msg);
	if (is_response)
	{
		//  TODO: ��ǰ��ΪHTTP��Ӧͷ�ڵ�һ�������п϶���������
		if (!IfResponseHasCompleteHeader(pkt_msg))
		{
			return false;
		}

		HttpStatusLine status_line;
		HttpHeader http_header;
		uint32 parse_len;
		if (!ParseStatusLineAndHeader(pkt_msg, status_line, 
			                          http_header, parse_len))
		{
			LOG(ERROR) << "Fail to parse http response";
			return false;
		}

		run_session->RcvdResponsePkt(status_line, http_header,
			L7_DATA(pkt_msg) + parse_len, L4_DATA_LEN(pkt_msg) - parse_len);
	}
	else
	{
		run_session->RcvdUnrecognizedPkt(
			L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));
	}

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: �Ƿ�����Ҫ�����HTTP����
 * ��  ��: [in] http_header ����ͷ
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpMonitor::IsInterestHttpRequest(const HttpHeader& http_header,
										const HttpRequestLine& request_line, HttpMatchType& type,
										std::string& regex_uri)
{
	// ����ͷ�е�Uri�Ƿ��������ļ���uriҪƥ��Ĺ���
	type = URI_MATCH_TYPE;
	if (IsInterestUri(request_line.uri, regex_uri)) return true;

	type = DOMAIN_MATCH_TYPE;
	// ������Ϣ����Ϣͷ���Ƿ����host��
	auto header_iter = http_header.find("host");
	if (header_iter == http_header.end()) return false;

	const std::string& host = header_iter->second;

	// �Ƿ��Ǹ���Ȥ��host�µ�uri
	if (IsInterestDomain(host, request_line.uri)) return true;

	DLOG(INFO) << "No match host or uri";
	
	return false;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����HTTP�����к�����ͷ
 * ��  ��: [in] pkt_msg ������Ϣ
 *         [in] request_line �����нṹ
 *         [in] http_header ����ͷ�ṹ
 *         [in] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpMonitor::ParseRequestLineAndHeader(const PktMsgSP& pkt_msg, 
	                                        HttpRequestLine& request_line, 
											HttpHeader& http_header,
											uint32& parse_len)
{
	char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	uint32 tmp_len = 0; parse_len = 0;

	//--- ����������

	HPN::RequestLineParseState rl_state = HPN::RLPS_INIT;
	if (!HPN::ParseHttpRequestLine(data, len, rl_state, request_line, tmp_len))
	{
		DLOG(ERROR) << "Fail to parse http request line";
		return false;
	}

	if (rl_state != HPN::RLPS_STOP) 
	{
		DLOG(ERROR) << "Incomplete http request line";
		return false;
	}

	if (tmp_len == len)
	{
		DLOG(WARNING) << "Http request only has request line";
		return false;
	}

	data += tmp_len; len -= tmp_len; parse_len += tmp_len;

	//--- ��������ͷ

	HPN::HttpHeaderParseState hh_state = HPN::HHPS_INIT;
	if (!HPN::ParseHttpHeader(data, len, hh_state, http_header, tmp_len))
	{
		DLOG(ERROR) << "Fail to parse http request header";
		return false;
	}

	if (hh_state != HPN::HHPS_STOP)
	{
		DLOG(ERROR) << "Incomplete http request header";
		return false;
	}

	if (tmp_len == len)
	{
		DLOG(WARNING) << "Http reqeust has no body";
	}
	
	parse_len += tmp_len;

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������Ӧ�к�HTTPͷ
 * ��  ��: [in] pkt_msg ������Ϣ
 *         [out] status_line ��Ӧ��
 *         [out] http_header HTTPͷ
 *         [out] parse_len �����ĳ���
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::ParseStatusLineAndHeader(const PktMsgSP& pkt_msg, 
										   HttpStatusLine& status_line, 
										   HttpHeader& http_header,
										   uint32& parse_len)
{
	char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	uint32 tmp_len = 0; parse_len = 0;

	//--- ����״̬��

	HPN::StatusLineParseState sl_state = HPN::SLPS_INIT;
	if (!ParseHttpStatusLine(data, len, sl_state, status_line, tmp_len))
	{
		DLOG(ERROR) << "Fail to parse http status line";
		return false;
	}

	if (sl_state != HPN::SLPS_STOP)
	{
		DLOG(ERROR) << "Incomplete status line";
		return false;
	}

	if (tmp_len == len)
	{
		DLOG(WARNING) << "Http response only has status line";
		return false;
	}

	data += tmp_len; len -= tmp_len; parse_len += tmp_len;

	//--- ��������ͷ

	HPN::HttpHeaderParseState hh_state = HPN::HHPS_INIT;
	if (!HPN::ParseHttpHeader(data, len, hh_state, http_header, tmp_len))
	{
		DLOG(ERROR) << "Fail to parse http response header";
		return false;
	}

	if (hh_state != HPN::HHPS_STOP)
	{
		DLOG(ERROR) << "Incomplete http response header";
		return false;
	}

	if (tmp_len == len)
	{
		DLOG(WARNING) << "Http response has no body";
	}

	parse_len += tmp_len;

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж����������Ƿ������������HTTPͷ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IfRequestHasCompleteHeader(const PktMsgSP& pkt_msg)
{
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж���Ӧ�������Ƿ������������HTTPͷ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IfResponseHasCompleteHeader(const PktMsgSP& pkt_msg)
{
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �½�RunSession
 * ��  ��: [in] conn_id ���ӱ�ʶ
 *         [in] request_line �����нṹ
 *         [in] http_header ����ͷ�ṹ
 * ����ֵ: ����ʱ�Ự
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
HttpRunSessionSP HttpMonitor::NewHttpRunSession(const ConnId& conn_id,
												const HttpRequestLine& request_line,
	                                            const HttpHeader& http_header,
	                                            const HttpMatchType type,
	                                            const std::string& match_uri)
{
	auto domain_iter = http_header.find("host");
	TMAS_ASSERT(domain_iter != http_header.end());
	if (domain_iter == http_header.end())
	{
		LOG(ERROR) << "Can not find host";
		return nullptr;
	}

	boost::mutex::scoped_lock lock(run_session_mutex_);

	// һ��TCP���ӣ�ͬһʱ��ֻ�ܴ���һ��HttpRunSession
	// ������HTTP����Ӧ�ø����Ѵ��ڵ�HttpRunSession
	auto iter = run_sessions_.find(conn_id);
	if (iter != run_sessions_.end())
	{
		DLOG(WARNING) << "New http session override old session";

		iter->second = HttpRunSessionSP(new HttpRunSession(this, 
			conn_id, http_header, domain_iter->second, request_line.uri, type, match_uri));
		
		return iter->second;
	}

	HttpRunSessionMap::value_type insert_value(conn_id, 
		HttpRunSessionSP(new HttpRunSession(this, conn_id, 
			http_header, domain_iter->second, request_line.uri, type, match_uri)));

	std::pair<HttpRunSessionMap::iterator, bool> insert_result;

	insert_result = run_sessions_.insert(insert_value);
	if (!insert_result.second)
	{
		LOG(ERROR) << "Fail to insert run session | " << conn_id;
		return nullptr;
	}

	return insert_result.first->second;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ự��ɴ���
 * ��  ��: [in] run_session ����ʱ�Ự
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HttpMonitor::SessionCompleted(const HttpRunSessionSP& run_session)
{
	DLOG(INFO) << "########## Session completed";

	const HttpRunSession::RunSessionInfo& info = run_session->GetRunSessionInfo();

	if (info.match_type == DOMAIN_MATCH_TYPE)
	{
		// ����domainͳ��
		HttpDomainStatSP domain_stat = GetHttpDomainStat(info.domain);
		if (!domain_stat)
		{
			LOG(ERROR) << "Can not get http domain stat | " << info.domain;
			return;
		}

		TMAS_ASSERT(info.response_time >= info.request_time);
		uint64 delay = info.request_time - info.response_time;

		domain_stat->total_session_num++;

		if (domain_stat->speed_monitor)
		{
			domain_stat->total_download_size += info.response_size;
		}
		if (domain_stat->delay_monitor)
		{
			domain_stat->total_response_delay += delay;
		}
	}
	else if (info.match_type == URI_MATCH_TYPE)
	{
		HttpUriStatSP uri_stat = GetHttpUriStat(info.match_uri);
		if (!uri_stat)
		{
			LOG(ERROR) << "Can not get http uri stat | " << info.match_uri;
			return;
		}

		TMAS_ASSERT(info.response_time >= info.request_time);
		uint64 delay = info.request_time - info.response_time;

		uri_stat->total_session_num++;

		if (uri_stat->speed_monitor)
		{
			uri_stat->total_download_size += info.response_size;
		}
		if (uri_stat->delay_monitor)
		{
			uri_stat->total_response_delay += delay;
		}
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ựʧ�ܴ���
 * ��  ��: [in] run_session ����ʱ�Ự
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HttpMonitor::SessionFailed(const HttpRunSessionSP& run_session)
{
	DLOG(INFO) << "Http session failed";
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡHTTP����Ϣ
 * ��  ��: [in] domain ����
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��02��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
HttpMonitor::HttpDomainStatSP
HttpMonitor::GetHttpDomainStat(const std::string& domain)
{
	auto domain_iter = domain_stat_.find(domain);
	if (domain_iter != domain_stat_.end())
	{
		return domain_iter->second;
	}

	TMAS_ASSERT(domain_iter == domain_stat_.end());

	// ����һ���µ�HttpDomainStat������

	HttpDomainStatMap::value_type insert_value(domain, HttpDomainStatSP(new HttpDomainStat));
	std::pair<HttpDomainStatMap::iterator, bool> insert_result;

	insert_result = domain_stat_.insert(insert_value);
	if (!insert_result.second)
	{
		LOG(ERROR) << "Fail to insert new domain stat | " << domain;
		return nullptr;
	}

	return insert_result.first->second;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡURIͳ�ƽṹ
 * ��  ��: [in] uri URI
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��20��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
HttpMonitor::HttpUriStatSP
HttpMonitor::GetHttpUriStat(const std::string& match_uri)
{
	auto uri_iter = uri_stat_.find(match_uri);
	if (uri_iter != uri_stat_.end())
	{
		return uri_iter->second;
	}

	TMAS_ASSERT(uri_iter == uri_stat_.end());

	// ����һ���µ�HttpDomainStat������

	HttpUriStatMap::value_type insert_value(match_uri, HttpUriStatSP(new HttpUriStat));
	std::pair<HttpUriStatMap::iterator, bool> insert_result;

	insert_result = uri_stat_.insert(insert_value);
	if (!insert_result.second)
	{
		LOG(ERROR) << "Fail to insert new domain stat | " << match_uri;
		return nullptr;
	}

	return insert_result.first->second;	
}


/*-----------------------------------------------------------------------------
 * ��  ��: TCP���ӹرպ󣬶�Ӧ��HTTP����Ҳ��Ҫɾ��
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HttpMonitor::ConnClosedProc(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	auto iter = run_sessions_.find(conn_id);
	if (iter == run_sessions_.end()) return;

	HttpRunSession::RunSessionState state = iter->second->GetRunSessionState();
	if (HttpRunSession::RSS_RECVED_RESPONSE != state)
	{
		DLOG(WARNING) << "Unexpected run session state " << state;
	}

	DLOG(INFO) << "Remove run session for connection closed | " << conn_id;

	run_sessions_.erase(iter);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж��Ƿ���HTTP����
 * ��  ��: [in] pkt_msg ������Ϣ
 *         [in][out] response_info ��Ӧ��Ϣ
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��02��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IsHttpRequest(const PktMsgSP& pkt_msg)
{
#if 0
	static boost::regex http_filter("^(GET|HEAD|PUT|DELETE|POST|OPTIONS) .*");

	return boost::regex_match(pkt_msg->l7_pkt_info.l7_data, http_filter);
#endif

	char* l7_data = pkt_msg->l7_pkt_info.l7_data;

	static uint32 get_method_bit = 0x20544547;

	if (*(reinterpret_cast<uint32*>(l7_data)) == get_method_bit)
	{
		return true;
	}

	return false;

}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж��Ƿ���HTTP��Ӧ
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��01��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IsHttpResponse(const PktMsgSP& pkt_msg)
{
#if 0
		static boost::regex http_filter("HTTP/\\d\\.\\d \\d{3} .*");
	
		return boost::regex_match(pkt_msg->l7_pkt_info.l7_data, http_filter);
#endif
		
	char* l7_data = pkt_msg->l7_pkt_info.l7_data;

	static uint32 response_bit = 0x50545448;

	if (*(reinterpret_cast<uint32*>(l7_data)) == response_bit)
	{
		return true;
	}

	return false;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж��Ƿ��Ǹ���Ȥ������
 * ��  ��: [in] domain ����
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IsInterestDomain(const std::string& domain, const std::string& uri)
{
	auto iter = host_site_map_.find(domain);
	if (iter == host_site_map_.end()) return false;

	//�ж��Ƿ�ƥ��Uri
	ParamsMap& params =  iter->second;
	auto params_iter = params.find("uri");

	//û����ƥ��ȫվ
	if (params_iter == params.end()) return true;
	
	if (!MatchUriPath(params_iter->second, uri)) return false;	

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: Uri·������ƥ��
 * ��  ��: [in] match_uri ����ƥ���ֶ� 
 *		 [in] uri  ��ƥ���ֶ�
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::MatchUriPath(const string match_uri, const string& uri)
{
    boost::smatch match;  // ������ʽƥ��Ľ��
    boost::regex reg(match_uri);  //ƥ�� key value

	bool match_result = false;
    if (boost::regex_match(uri, match, reg))  // ������ʽƥ��
    {
		match_result = true;
    }

    return match_result;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж��Ƿ��Ǹ���Ȥ��URI
 * ��  ��: [in] uri URI
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IsInterestUri(const std::string uri, std::string& regex_uri)
{

	DLOG(WARNING) << "uri: " << uri << " regex_uri: " << regex_uri;
	
	for (auto uri_stat : uri_site_map_)
	{
		if (MatchUriPath(uri_stat.first, uri))
		{
			regex_uri = uri_stat.first;
			DLOG(WARNING) << "Uri match success";
			return true;
		}			
	}

	return false;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����URI�����������ٶȵ��̺߳���
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014.02.24
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
void HttpMonitor::SpeedTestProc()
{
	HttpDomainStatMap domain_stats;

	domain_stat_mutex_.lock();
	domain_stats.swap(domain_stat_); 
	domain_stat_mutex_.unlock();

	std::ofstream ofs("http_speed.log", std::ios_base::app);

	//�������ʱ��������
	for (const std::pair<std::string, HttpDomainStatSP>& p : domain_stats)
	{
		HttpDomainStat& domain_stat = *p.second; 

		if (domain_stat.delay_monitor || domain_stat.speed_monitor) break;

		if (GetDurationMilliSeconds(domain_stat.update_time, TimeNow()) < domain_stat.stat_interval) continue;
		domain_stat.update_time = TimeNow();

		if (domain_stat.total_session_num == 0
			|| domain_stat.total_response_delay == 0)
		{
			continue;
		}

		// ��¼������ƽ��ʱ�Ӻ�ƽ�������ٶ�
		if (domain_stat.delay_monitor && domain_stat.speed_monitor)
		{
			ofs << p.first 
				<< " " 
				<< domain_stat.total_response_delay / domain_stat.total_session_num 
				<< " "
				<< domain_stat.total_download_size * 1000 / domain_stat.total_response_delay
				<< std::endl;
		}
		else if (domain_stat.delay_monitor)
		{
			ofs << p.first 
				<< " " 
				<< domain_stat.total_response_delay / domain_stat.total_session_num 
				<< std::endl;

		}
		else if (domain_stat.speed_monitor)
		{
			ofs << p.first 
				<< "        " 
				<< domain_stat.total_download_size * 1000 / domain_stat.total_response_delay
				<< std::endl;
		}
		
		ofs << '\n';
	}

	HttpUriStatMap uri_stats;

	uri_stat_mutex_.lock();
	uri_stats.swap(uri_stat_); 
	uri_stat_mutex_.unlock();
	for (const std::pair<std::string, HttpUriStatSP>& p : uri_stats)
	{
		HttpUriStat& uri_stat = *p.second; 

		if (uri_stat.delay_monitor || uri_stat.speed_monitor) break;

		if (GetDurationMilliSeconds(uri_stat.update_time, TimeNow()) < uri_stat.stat_interval) continue;
		uri_stat.update_time = TimeNow();

		if (uri_stat.total_session_num == 0
			|| uri_stat.total_response_delay == 0)
		{
			continue;
		}
			
		// ��¼������ƽ��ʱ�Ӻ�ƽ�������ٶ�
		if (uri_stat.delay_monitor && uri_stat.speed_monitor)
		{
			ofs << p.first 
				<< " " 
				<< uri_stat.total_response_delay / uri_stat.total_session_num 
				<< " "
				<< uri_stat.total_download_size * 1000 / uri_stat.total_response_delay
				<< std::endl;
		}
		else if (uri_stat.delay_monitor)
		{
			ofs << p.first 
				<< " " 
				<< uri_stat.total_response_delay / uri_stat.total_session_num 
				<< std::endl;

		}
		else if (uri_stat.speed_monitor)
		{
			ofs << p.first 
				<< "        " 
				<< uri_stat.total_download_size * 1000 / uri_stat.total_response_delay
				<< std::endl;
		}

		ofs << '\n';

	}

	ofs.close();
}

}
