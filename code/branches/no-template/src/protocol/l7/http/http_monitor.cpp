/*#############################################################################
 * 文件名   : http_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月12日
 * 文件描述 : HttpMonitor类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: HttpMonitor类的构造函数
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014.02.24
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
HttpMonitor::HttpMonitor()
{
}

/*------------------------------------------------------------------------------
 * 描  述: HttpMonitor类的析构函数
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014.02.24
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
HttpMonitor::~HttpMonitor()
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 加载配置文件
 * 参  数: 
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpMonitor::LoadConfiguration()
{
    bool ok = false;
	HttpConfigParser parser;
    ok = parser.Parse("../config/http.conf", host_site_map_, uri_site_map_);
	
    return ok;
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 详见pkt_processor.hpp
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 处理HTTP请求
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月11日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 处理RunSession
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 是否处理了报文
 * 修  改:
 *   时间 2014年03月01日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpMonitor::TryProcHttpRunSession(const PktMsgSP& pkt_msg)
{
	DLOG(WARNING) << "Try to process http run session";

	boost::mutex::scoped_lock lock(run_session_mutex_);

	//RunSession不存在，则没必要再往下处理了，即使是HttpResponse
	auto iter = run_sessions_.find(CONN_ID(pkt_msg));
	if (iter == run_sessions_.end()) return false;

	HttpRunSessionSP& run_session = iter->second;

	bool is_response = IsHttpResponse(pkt_msg);
	if (is_response)
	{
		//  TODO: 当前认为HTTP响应头在第一个报文中肯定是完整的
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
 * 描  述: 是否是需要处理的HTTP请求
 * 参  数: [in] http_header 请求头
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpMonitor::IsInterestHttpRequest(const HttpHeader& http_header,
										const HttpRequestLine& request_line, HttpMatchType& type,
										std::string& regex_uri)
{
	// 请求头中的Uri是否是配置文件中uri要匹配的规则
	type = URI_MATCH_TYPE;
	if (IsInterestUri(request_line.uri, regex_uri)) return true;

	type = DOMAIN_MATCH_TYPE;
	// 请求消息的消息头中是否包含host域
	auto header_iter = http_header.find("host");
	if (header_iter == http_header.end()) return false;

	const std::string& host = header_iter->second;

	// 是否是感兴趣的host下的uri
	if (IsInterestDomain(host, request_line.uri)) return true;

	DLOG(INFO) << "No match host or uri";
	
	return false;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析HTTP请求行和请求头
 * 参  数: [in] pkt_msg 报文消息
 *         [in] request_line 请求行结构
 *         [in] http_header 请求头结构
 *         [in] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpMonitor::ParseRequestLineAndHeader(const PktMsgSP& pkt_msg, 
	                                        HttpRequestLine& request_line, 
											HttpHeader& http_header,
											uint32& parse_len)
{
	char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	uint32 tmp_len = 0; parse_len = 0;

	//--- 解析请求行

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

	//--- 解析请求头

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
 * 描  述: 解析响应行和HTTP头
 * 参  数: [in] pkt_msg 报文消息
 *         [out] status_line 响应行
 *         [out] http_header HTTP头
 *         [out] parse_len 解析的长度
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月04日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpMonitor::ParseStatusLineAndHeader(const PktMsgSP& pkt_msg, 
										   HttpStatusLine& status_line, 
										   HttpHeader& http_header,
										   uint32& parse_len)
{
	char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	uint32 tmp_len = 0; parse_len = 0;

	//--- 解析状态行

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

	//--- 解析请求头

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
 * 描  述: 判断请求报文中是否包含了完整的HTTP头
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月04日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IfRequestHasCompleteHeader(const PktMsgSP& pkt_msg)
{
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 判断响应报文中是否包含了完整的HTTP头
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月04日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IfResponseHasCompleteHeader(const PktMsgSP& pkt_msg)
{
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 新建RunSession
 * 参  数: [in] conn_id 连接标识
 *         [in] request_line 请求行结构
 *         [in] http_header 请求头结构
 * 返回值: 运行时会话
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
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

	// 一个TCP连接，同一时刻只能存在一个HttpRunSession
	// 新来的HTTP请求应该覆盖已存在的HttpRunSession
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
 * 描  述: 会话完成处理
 * 参  数: [in] run_session 运行时会话
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void HttpMonitor::SessionCompleted(const HttpRunSessionSP& run_session)
{
	DLOG(INFO) << "########## Session completed";

	const HttpRunSession::RunSessionInfo& info = run_session->GetRunSessionInfo();

	if (info.match_type == DOMAIN_MATCH_TYPE)
	{
		// 更新domain统计
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
 * 描  述: 会话失败处理
 * 参  数: [in] run_session 运行时会话
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void HttpMonitor::SessionFailed(const HttpRunSessionSP& run_session)
{
	DLOG(INFO) << "Http session failed";
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取HTTP域信息
 * 参  数: [in] domain 域名
 * 返回值: 
 * 修  改: 
 *   时间 2014年02月11日
 *   作者 teck_zhou
 *   描述 创建
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

	// 创建一个新的HttpDomainStat并插入

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
 * 描  述: 获取URI统计结构
 * 参  数: [in] uri URI
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月20日
 *   作者 tom_liu
 *   描述 创建
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

	// 创建一个新的HttpDomainStat并插入

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
 * 描  述: TCP连接关闭后，对应的HTTP数据也需要删除
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 判断是否是HTTP请求
 * 参  数: [in] pkt_msg 报文消息
 *         [in][out] response_info 响应信息
 * 返回值: 是/否
 * 修  改: 
 *   时间 2014年02月11日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 判断是否是HTTP响应
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 是/否
 * 修  改: 
 *   时间 2014年01月12日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 判断是否是感兴趣的域名
 * 参  数: [in] domain 域名
 * 返回值: 是/否
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpMonitor::IsInterestDomain(const std::string& domain, const std::string& uri)
{
	auto iter = host_site_map_.find(domain);
	if (iter == host_site_map_.end()) return false;

	//判断是否匹配Uri
	ParamsMap& params =  iter->second;
	auto params_iter = params.find("uri");

	//没有则匹配全站
	if (params_iter == params.end()) return true;
	
	if (!MatchUriPath(params_iter->second, uri)) return false;	

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: Uri路径正则匹配
 * 参  数: [in] match_uri 规则匹配字段 
 *		 [in] uri  被匹配字段
 * 返回值: 是/否
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpMonitor::MatchUriPath(const string match_uri, const string& uri)
{
    boost::smatch match;  // 正则表达式匹配的结果
    boost::regex reg(match_uri);  //匹配 key value

	bool match_result = false;
    if (boost::regex_match(uri, match, reg))  // 正则表达式匹配
    {
		match_result = true;
    }

    return match_result;
}

/*-----------------------------------------------------------------------------
 * 描  述: 判断是否是感兴趣的URI
 * 参  数: [in] uri URI
 * 返回值: 是/否
 * 修  改: 
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 测试URI或是域名的速度的线程函数
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014.02.24
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
void HttpMonitor::SpeedTestProc()
{
	HttpDomainStatMap domain_stats;

	domain_stat_mutex_.lock();
	domain_stats.swap(domain_stat_); 
	domain_stat_mutex_.unlock();

	std::ofstream ofs("http_speed.log", std::ios_base::app);

	//做输出和时间间隔处理
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

		// 记录域名的平均时延和平均下载速度
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
			
		// 记录域名的平均时延和平均下载速度
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
