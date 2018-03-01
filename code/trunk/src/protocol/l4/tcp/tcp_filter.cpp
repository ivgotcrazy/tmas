/*#############################################################################
 * 文件名   : tcp_filter.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : TcpFilter类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "tcp_filter.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 确定连接是否匹配过滤器，只要有一个不匹配就返回false。
 * 参  数: [in] conn_id 连接标识
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpFilter::Match(const TcpRecordInfo& record_info)
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
 * 参  数: [in] ip_regions 从配置文件解析出来的IP段
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
TcpIpFilter::TcpIpFilter(const IpRegionVec& ip_regions)
	: ip_regions_(ip_regions)
{
	TMAS_ASSERT(!ip_regions_.empty());
}

/*------------------------------------------------------------------------------
 * 描  述: 确定连接是否匹配过滤器，源/目的IP地址只要有一个匹配就OK。
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpIpFilter::DoMatch(const TcpRecordInfo& record_info)
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
 * 参  数: [in] port_region 从配置文件解析出来的端口段
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
TcpPortFilter::TcpPortFilter(const PortRegionVec& port_regions)
	: port_regions_(port_regions)
{
	TMAS_ASSERT(!port_regions_.empty());
}

/*------------------------------------------------------------------------------
 * 描  述: 确定连接是否匹配过滤器，源/目的端口只要有一个配置就OK。
 * 参  数: [in] record_info 记录信息
 * 返回值: 是否匹配过滤器
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpPortFilter::DoMatch(const TcpRecordInfo& record_info)
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

}