/*#############################################################################
 * 文件名   : http_filter.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : HttpFiltr相关类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_FILTER
#define BROADINTER_HTTP_FILTER

#include <string>
#include <boost/regex.hpp>
#include <boost/noncopyable.hpp>

#include "http_typedef.hpp"
#include "ip_region_parser.hpp"
#include "port_region_parser.hpp"

namespace BroadInter
{

using std::string;

/******************************************************************************
 * 描述: HTTP过滤器基类
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpFilter : public boost::noncopyable
{
public:
	bool Match(const HttpRecordInfo& record_info);

	void SetDecorator(const HttpFilterSP& filter) { decorator_ = filter; }

private:
	virtual bool DoMatch(const HttpRecordInfo& record_info) = 0;

private:
	HttpFilterSP decorator_;
};

/******************************************************************************
 * 描述: IP地址过滤器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpIpFilter : public HttpFilter
{
public:
	HttpIpFilter(const IpRegionVec& ip_regions);

private:
	virtual bool DoMatch(const HttpRecordInfo& record_info) override;

private:
	IpRegionVec ip_regions_;
};

/******************************************************************************
 * 描述: 端口过滤器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpPortFilter : public HttpFilter
{
public:
	HttpPortFilter(const PortRegionVec& port_regions);

private:
	virtual bool DoMatch(const HttpRecordInfo& record_info) override;

private:
	PortRegionVec port_regions_;
};

/******************************************************************************
 * 描述: Host字段过滤器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpHostFilter : public HttpFilter
{
public:
	HttpHostFilter(const string& regex);
	
private:
	virtual bool DoMatch(const HttpRecordInfo& record_info) override;

private:
	boost::regex host_regex_;
};

/******************************************************************************
 * 描述: URI过滤器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpUriFilter : public HttpFilter
{
public:
	HttpUriFilter(const string& regex);

private:
	virtual bool DoMatch(const HttpRecordInfo& record_info) override;

private:
	boost::regex uri_regex_;
};

}

#endif