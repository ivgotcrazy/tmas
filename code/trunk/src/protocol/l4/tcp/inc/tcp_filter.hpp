/*#############################################################################
 * 文件名   : tcp_filter.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : TcpFilter类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_FILTER
#define BROADINTER_TCP_FILTER

#include <boost/noncopyable.hpp>

#include "connection.hpp"
#include "tcp_typedef.hpp"
#include "ip_region_parser.hpp"
#include "port_region_parser.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: TCP过滤器基类
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpFilter : public boost::noncopyable
{
public:
	bool Match(const TcpRecordInfo& record_info);

	void SetDecorator(const TcpFilterSP& filter) { decorator_ = filter; }

private:
	virtual bool DoMatch(const TcpRecordInfo& record_info) = 0;

private:
	TcpFilterSP decorator_;
};

/******************************************************************************
 * 描述: IP地址过滤器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpIpFilter : public TcpFilter
{
public:
	TcpIpFilter(const IpRegionVec& ip_regions);

private:
	virtual bool DoMatch(const TcpRecordInfo& record_info) override;

private:
	IpRegionVec ip_regions_;
};

/******************************************************************************
 * 描述: 端口过滤器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpPortFilter : public TcpFilter
{
public:
	TcpPortFilter(const PortRegionVec& port_regions);

private:
	virtual bool DoMatch(const TcpRecordInfo& record_info) override;

private:
	PortRegionVec port_regions_;
};

}

#endif