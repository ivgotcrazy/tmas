/*#############################################################################
 * 文件名   : http_filter.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : HttpFiltr相关类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_filter.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 确定HTTP会话是否匹配过滤器，只要有一个不匹配就返回false。
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpFilter::Match(const HttpRecordInfo& record_info)
{
	if (!DoMatch(record_info)) return false;

	if (decorator_)
	{
		return decorator_->Match(record_info);
	}
	else
	{
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] ip_regions IP地址段
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpIpFilter::HttpIpFilter(const IpRegionVec& ip_regions) 
	: ip_regions_(ip_regions)
{
	TMAS_ASSERT(!ip_regions_.empty());
}

/*------------------------------------------------------------------------------
 * 描  述: 确定HTTP会话是否匹配IP地址过滤器
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpIpFilter::DoMatch(const HttpRecordInfo& record_info)
{
	const ConnId& conn_id = record_info.conn_id;

	for (const IpRegion& ip_region : ip_regions_)
	{
		if (ip_region.start_addr <= conn_id.smaller_ip 
			&& conn_id.smaller_ip <= ip_region.end_addr)
		{
			return true;
		}

		if (ip_region.start_addr <= conn_id.bigger_ip 
			&& conn_id.bigger_ip <= ip_region.end_addr)
		{
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] port_regions 端口段
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpPortFilter::HttpPortFilter(const PortRegionVec& port_regions) 
	: port_regions_(port_regions)
{
	TMAS_ASSERT(!port_regions_.empty());
}

/*------------------------------------------------------------------------------
 * 描  述: 确定HTTP会话是否匹配端口过滤器
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpPortFilter::DoMatch(const HttpRecordInfo& record_info)
{
	const ConnId& conn_id = record_info.conn_id;

	for (const PortRegion& port_region : port_regions_)
	{
		if (port_region.start_port <= conn_id.smaller_port
			&& conn_id.smaller_port <= port_region.end_port)
		{
			return true;
		}

		if (port_region.start_port <= conn_id.bigger_port
			&& conn_id.bigger_port <= port_region.end_port)
		{
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] regex 正则表达式字符串
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpHostFilter::HttpHostFilter(const string& regex)
	: host_regex_(boost::regex(regex))
{
	if (regex.empty())
	{
		LOG(WARNING) << "Empty host regex";
	}
}

/*------------------------------------------------------------------------------
 * 描  述: 确定HTTP会话是否匹配Host过滤器
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpHostFilter::DoMatch(const HttpRecordInfo& record_info)
{
	const HttpRequest& request = record_info.request_info;

	// 如果请求头中没有"host"字段，匹配失败
	auto host_iter = request.header.find("host");
	if (host_iter == request.header.end())
	{
		DLOG(WARNING) << "Can not find host in http request";
		return false;
	}

	return boost::regex_match(host_iter->second, host_regex_);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] regex 正则表达式字符串
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpUriFilter::HttpUriFilter(const string& regex)
	: uri_regex_(boost::regex(regex))
{
	if (regex.empty())
	{
		LOG(WARNING) << "Empty uri regex";
	}
}

/*------------------------------------------------------------------------------
 * 描  述: 确定HTTP会话是否匹配URI过滤器
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpUriFilter::DoMatch(const HttpRecordInfo& record_info)
{
	const string& uri = record_info.request_info.request_line.uri;
	return boost::regex_match(uri, uri_regex_);
}


}