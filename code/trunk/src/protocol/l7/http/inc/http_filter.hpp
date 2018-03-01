/*#############################################################################
 * �ļ���   : http_filter.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : HttpFiltr���������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: HTTP����������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: IP��ַ������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: �˿ڹ�����
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: Host�ֶι�����
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: URI������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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