/*#############################################################################
 * �ļ���   : tcp_filter.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : TcpFilter������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: TCP����������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: IP��ַ������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: �˿ڹ�����
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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