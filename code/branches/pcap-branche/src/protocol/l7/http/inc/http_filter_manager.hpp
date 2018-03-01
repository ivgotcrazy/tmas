/*#############################################################################
 * 文件名   : http_filter_manager.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月30日
 * 文件描述 : HttpFilterManager类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_FILTER_MANAGER
#define BROADINTER_HTTP_FILTER_MANAGER

#include <vector>
#include <boost/regex.hpp>

#include "http_typedef.hpp"
#include "http_recorder.hpp"
#include "http_config_parser.hpp"
#include "timer.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * 描  述: 过滤器基类
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class HttpFilter
{
public:
	HttpFilter(HttpRecorder* recorder) : recorder_(recorder) {}
	virtual ~HttpFilter() { delete recorder_; }

	virtual bool MatchHttpRequest(const HttpRequest& request,
											const uint32 smaller_ip, const uint32 bigger_ip) = 0;
	virtual bool MatchIpRegion(const uint32 src_addr, const uint32 dst_addr) = 0;

	HttpRecorder* GetHttpRecorder() const { return recorder_; }

private:
	HttpRecorder* recorder_;
};

/*******************************************************************************
 * 描  述: URI过滤器
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class UriFilter : public HttpFilter
{
public:
	UriFilter(const string& uri, const IpAddrVec& src_ip, const IpAddrVec& dst_ip, 
				HttpRecorder* recorder);
	
	virtual bool MatchHttpRequest(const HttpRequest& request,
									const uint32 smaller_ip, const uint32 bigger_ip) override;

	bool MatchIpRegion(const uint32 src_addr, const uint32 dst_addr) override;

private:
	boost::regex uri_regex_;
	IpAddrVec src_ips_;
	IpAddrVec dst_ips_;
};

/*******************************************************************************
 * 描  述: Host过滤器
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class HostFilter : public HttpFilter
{
public:
	typedef std::vector<string> UriVec;

	HostFilter(const string& host, const UriVec& uris, const IpAddrVec& src_ip,
					const IpAddrVec& dst_ip, HttpRecorder* recorder);

	virtual bool MatchHttpRequest(const HttpRequest& request,
									const uint32 smaller_ip, const uint32 bigger_ip) override;

	bool MatchIpRegion(const uint32 src_addr, const uint32 dst_addr) override;

private:
	boost::regex host_regex_;
	std::vector<boost::regex> uri_regexs_;
	IpAddrVec src_ips_;
	IpAddrVec dst_ips_;
};

/*******************************************************************************
 * 描  述: 过滤器管理
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class HttpFilterManager
{
public:
	typedef std::vector<HttpRecorder*> RecorderVec;

	~HttpFilterManager();

	bool Init();

	void GetMatchedRecorders(const HttpRequest& request, 
										const uint32 smaller_ip, const uint32 bigger_ip,
										RecorderVec& recorders);

private:
	void OnTick();

private:
	std::vector<HostFilter*> host_filters_;
	std::vector<UriFilter*> uri_filters_;

	boost::scoped_ptr<FreeTimer> record_timer_;
};

}

#endif