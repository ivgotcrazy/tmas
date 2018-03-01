/*#############################################################################
 * �ļ���   : http_filter.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : HttpFiltr�����ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_filter.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ȷ��HTTP�Ự�Ƿ�ƥ���������ֻҪ��һ����ƥ��ͷ���false��
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] ip_regions IP��ַ��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpIpFilter::HttpIpFilter(const IpRegionVec& ip_regions) 
	: ip_regions_(ip_regions)
{
	TMAS_ASSERT(!ip_regions_.empty());
}

/*------------------------------------------------------------------------------
 * ��  ��: ȷ��HTTP�Ự�Ƿ�ƥ��IP��ַ������
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] port_regions �˿ڶ�
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpPortFilter::HttpPortFilter(const PortRegionVec& port_regions) 
	: port_regions_(port_regions)
{
	TMAS_ASSERT(!port_regions_.empty());
}

/*------------------------------------------------------------------------------
 * ��  ��: ȷ��HTTP�Ự�Ƿ�ƥ��˿ڹ�����
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] regex ������ʽ�ַ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ȷ��HTTP�Ự�Ƿ�ƥ��Host������
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpHostFilter::DoMatch(const HttpRecordInfo& record_info)
{
	const HttpRequest& request = record_info.request_info;

	// �������ͷ��û��"host"�ֶΣ�ƥ��ʧ��
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
 * ��  ��: ���캯��
 * ��  ��: [in] regex ������ʽ�ַ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ȷ��HTTP�Ự�Ƿ�ƥ��URI������
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpUriFilter::DoMatch(const HttpRecordInfo& record_info)
{
	const string& uri = record_info.request_info.request_line.uri;
	return boost::regex_match(uri, uri_regex_);
}


}