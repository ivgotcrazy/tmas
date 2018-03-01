/*#############################################################################
 * 文件名   : http_filter_manager.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月31日
 * 文件描述 : HttpFilterManager类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_filter_manager.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] uri URI正则表达式
 *         [in] recoder 对应的记录器
 * 返回值: 
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
UriFilter::UriFilter(const string& uri, const IpAddrVec& src_ip, const IpAddrVec& dst_ip, HttpRecorder* recorder)
	: HttpFilter(recorder)
	, uri_regex_(boost::regex(uri))
	, src_ips_(src_ip)
	, dst_ips_(dst_ip)
{

}

/*-----------------------------------------------------------------------------
 * 描  述: 匹配HTTP请求
 * 参  数: [in] request HTTP请求
 * 返回值: 匹配结果:成功/失败
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool UriFilter::MatchHttpRequest(const HttpRequest& request,
										const uint32 smaller_ip, const uint32 bigger_ip)
{
	if (!boost::regex_match(request.request_line.uri, uri_regex_)) return false;

	//由于目前不支持原地址和目标地址，现在只要地址对匹配成功就行
	if ((!MatchIpRegion(smaller_ip, bigger_ip)) && (!MatchIpRegion(bigger_ip, smaller_ip)))
	{
		DLOG(INFO) << "Match uri ip region failed";
		return false;
	}	

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 匹配Ip地址段
 * 参  数: [in] src_addr 源地址
 *         [in] dst_addr 目标地址
 * 返回值: 匹配结果:成功/失败
 * 修  改:
 *   时间 2014年04月17日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool UriFilter::MatchIpRegion(const uint32 src_addr, const uint32 dst_addr)
{
	bool ret = false;
	for (auto src_region : src_ips_)
	{
		if (src_region.start_addr <= src_addr &&  src_addr <= src_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth uri src ip region failed";
		return false;
	}

	ret = false;
	for (auto dst_region : dst_ips_)
	{
		if (dst_region.start_addr <= dst_addr &&  dst_addr <= dst_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth uri dst ip region failed";
		return false;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] host Host正则表达式
 *         [in] uris 域名下URI过滤
 *         [in] recoder 对应的记录器
 * 返回值: 
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
HostFilter::HostFilter(const string& host, const UriVec& uris, const IpAddrVec& src_ip, 
							const IpAddrVec& dst_ip, HttpRecorder* recorder)
	: HttpFilter(recorder)
	, host_regex_(boost::regex(host))
	, src_ips_(src_ip)
	, dst_ips_(dst_ip)
{
	for (const string& uri : uris)
	{
		uri_regexs_.push_back(boost::regex(uri));
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 匹配Ip地址段
 * 参  数: [in] src_addr 源地址
 *         [in] dst_addr 目标地址
 * 返回值: 匹配结果:成功/失败
 * 修  改:
 *   时间 2014年04月17日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HostFilter::MatchHttpRequest(const HttpRequest& request, const uint32 smaller_ip, 
											const uint32 bigger_ip)
{
	// 如果请求头中没有"host"字段，匹配失败
	auto host_iter = request.header.find("host");
	if (host_iter == request.header.end())
	{
		DLOG(WARNING) << "Can not find host in http request";
		return false;
	}

	bool ret = false;
	if (boost::regex_match(host_iter->second, host_regex_))
	{
		if (uri_regexs_.empty())
		{
			ret = true;
		}
		else
		{
			for (const boost::regex& uri_regex : uri_regexs_)
			{
				if (boost::regex_match(request.request_line.uri, uri_regex))
				{
					ret = true;
					break;
				}
			}
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Match Host or uri failed";
		return false;
	}
	
	//由于目前不支持原地址和目标地址，现在只要地址对匹配成功就行
	if ((!MatchIpRegion(smaller_ip, bigger_ip)) && (!MatchIpRegion(bigger_ip, smaller_ip)))
	{
		DLOG(INFO) << "Match host ip region failed";
		return false;
	}
	
	return ret;
}

/*-----------------------------------------------------------------------------
 * 描  述: 匹配Ip地址段
 * 参  数: [in] request HTTP请求
 * 返回值: 匹配结果:成功/失败
 * 修  改:
 *   时间 2014年04月17日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HostFilter::MatchIpRegion(const uint32 src_addr, const uint32 dst_addr)
{	
	bool ret = false;
	for (auto src_region : src_ips_)
	{
		if (src_region.start_addr <= src_addr &&  src_addr <= src_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth host src ip region failed";
		return false;
	}

	ret = false;
	for (auto dst_region : dst_ips_)
	{
		if (dst_region.start_addr <= dst_addr &&  dst_addr <= dst_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth host dst ip region failed";
		return false;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * 描  述: 析构函数
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
HttpFilterManager::~HttpFilterManager()
{
	record_timer_->Stop();

	for (HostFilter* filter : host_filters_)
	{
		delete filter;
	}

	for (UriFilter* filter : uri_filters_)
	{
		delete filter;
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数:
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HttpFilterManager::Init()
{
	HostConfigVec host_configs;
	UriConfigVec uri_configs;

	if (!HttpConfigParser::ParseConfig(host_configs, uri_configs))
	{
		LOG(ERROR) << "Fail to parse http config";
		return false;
	}

	for (const HostConfig& host_config : host_configs)
	{
		HostRecorder* recorder = 
			new HostRecorder(host_config.host, host_config.delay_monitor, 
				host_config.speed_monitor, host_config.stat_interval);

		host_filters_.push_back(
			new HostFilter(host_config.host, host_config.uris, 
							host_config.src_ips, host_config.dst_ips, recorder));
	}

	for (const UriConfig& uri_config : uri_configs)
	{
		UriRecorder* recorder = 
			new UriRecorder(uri_config.uri, uri_config.delay_monitor, 
				uri_config.speed_monitor, uri_config.stat_interval);

		uri_filters_.push_back(new UriFilter(uri_config.uri,
								uri_config.src_ips, uri_config.dst_ips, recorder));
	}

	record_timer_.reset(new FreeTimer(
		boost::bind(&HttpFilterManager::OnTick, this), 1));

	record_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 定时器处理
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void HttpFilterManager::OnTick()
{
	for (HostFilter* filter : host_filters_)
	{
		filter->GetHttpRecorder()->OnTick();
	}

	for (UriFilter* filter : uri_filters_)
	{
		filter->GetHttpRecorder()->OnTick();
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取请求对应的过滤器
 * 参  数: [in] request HTTP请求
 *         [out] recorders 相关记录器
 * 返回值: 
 * 修  改:
 *   时间 2014年03月31日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void HttpFilterManager::GetMatchedRecorders(const HttpRequest& request,
													const uint32 smaller_ip, 
													const uint32 bigger_ip,
													RecorderVec	& recorders)
{
	// 先匹配URI规则
	for (UriFilter* filter : uri_filters_)
	{
		if (filter->MatchHttpRequest(request, smaller_ip, bigger_ip))
		{
			recorders.push_back(filter->GetHttpRecorder());
		}
	}

	// 再匹配Host规则
	for (HostFilter* filter : host_filters_)
	{
		if (filter->MatchHttpRequest(request, smaller_ip, bigger_ip))
		{
			recorders.push_back(filter->GetHttpRecorder());
		}
	}
}

}