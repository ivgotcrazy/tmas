/*#############################################################################
 * �ļ���   : http_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : HttpMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_MONITOR
#define BROADINTER_HTTP_MONITOR

#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"

#include "connection.hpp"
#include "timer.hpp"
#include "http_typedef.hpp"
#include "http_config_parser.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: HTTP���ݼ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class HttpMonitor : public PktProcessor
{
public:
    HttpMonitor();
    virtual ~HttpMonitor();

	bool Init();

	// RunSession֪ͨMonitor�Ự�Ѿ����
	void SessionCompleted(const HttpRunSessionSP& run_session);

	// RunSession֪ͨMonitro�Ự������
	void SessionFailed(const HttpRunSessionSP& run_session);

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	//---------------------------------------------------------------	
	//ƥ�������ļ���uri�ֶε�Uri
	struct HttpUriStat
	{
		HttpUriStat() : total_session_num(0), total_download_size(0)
						, total_response_delay(0), delay_monitor(false)
						, speed_monitor(false), stat_interval(20) 
						, update_time(TimeNow())
		{
		}
	
		uint64 total_session_num;		// ���ӵĸ���
		uint64 total_download_size;		// �����ش�С
		uint64 total_response_delay;	// ����Ӧ��ʱ
		bool delay_monitor;
		bool speed_monitor;
		uint32 stat_interval;
		
		ptime update_time;
	};

	typedef boost::shared_ptr<HttpUriStat> HttpUriStatSP;
	typedef boost::unordered_map<std::string, HttpUriStatSP> HttpUriStatMap;
	
	//---------------------------------------------------------------

	struct HttpDomainStat // ָ��domain��ͳ������
	{
		HttpDomainStat() : total_session_num(0)
			, total_download_size(0), total_response_delay(0)
			, delay_monitor(false), speed_monitor(false)
			, stat_interval(20) 
		{
			update_time = TimeNow();
		}

		uint64 total_session_num;
		uint64 total_download_size;
		uint64 total_response_delay;
		bool delay_monitor;
		bool speed_monitor;
		uint32 stat_interval;

		ptime update_time;
	};

	typedef boost::shared_ptr<HttpDomainStat> HttpDomainStatSP;
	typedef boost::unordered_map<std::string, HttpDomainStatSP> HttpDomainStatMap;
	
	//---------------------------------------------------------------

	typedef boost::unordered_map<ConnId, HttpRunSessionSP> HttpRunSessionMap;
	typedef HttpRunSessionMap::iterator RunSessionIter;

	typedef boost::unordered_set<std::string> HttpInterestUriSet;
	typedef boost::unordered_map<std::string, HttpInterestUriSet> HttpInterestDomainMap;

private:
	bool LoadConfiguration();
	
	bool IsHttpRequest(const PktMsgSP& pkt_msg);
	bool IsHttpResponse(const PktMsgSP& pkt_msg);

	bool MatchUriPath(const std::string match_uri, const std::string& uri);
	bool IsInterestDomain(const std::string& domain, const std::string& uri);
	bool IsInterestUri(const std::string uri, std::string& regex_uri);
	bool IsInterestHttpRequest(const HttpHeader& http_header,
									const HttpRequestLine& request_line, HttpMatchType& type,
									std::string& regex_uri);

	bool IfRequestHasCompleteHeader(const PktMsgSP& pkt_msg);
	bool IfResponseHasCompleteHeader(const PktMsgSP& pkt_msg);

	void ProcessHttpRequest(const PktMsgSP& pkt_msg);
	bool TryProcHttpRunSession(const PktMsgSP& pkt_msg);
	
	bool ParseRequestLineAndHeader(
		const PktMsgSP& pkt_msg, 
		HttpRequestLine& request_line, 
		HttpHeader& http_header,
		uint32& parse_len);

	bool ParseStatusLineAndHeader(
		const PktMsgSP& pkt_msg, 
		HttpStatusLine& status_line, 
		HttpHeader& http_header,
		uint32& parse_len);

	HttpRunSessionSP NewHttpRunSession(
		const ConnId& conn_id,
		const HttpRequestLine& request_line,
	    const HttpHeader& http_header,
	    const HttpMatchType type,
	    const std::string& match_uri);
	
	void ConnClosedProc(const ConnId& conn_id);
 
	HttpDomainStatSP GetHttpDomainStat(const std::string& domain);

	HttpUriStatSP GetHttpUriStat(const std::string& uri);

	void SpeedTestProc();

private:
	SiteParamsMap host_site_map_;
	SiteParamsMap uri_site_map_;

	//HttpInterestDomainMap interest_domains_;

	HttpRunSessionMap run_sessions_;
	boost::mutex run_session_mutex_;

	HttpDomainStatMap domain_stat_;
	boost::mutex domain_stat_mutex_;

	HttpUriStatMap uri_stat_;
	boost::mutex uri_stat_mutex_;
	
	boost::scoped_ptr<FreeTimer> speed_test_timer_;
};

}

#endif
