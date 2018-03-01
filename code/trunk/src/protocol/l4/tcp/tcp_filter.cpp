/*#############################################################################
 * �ļ���   : tcp_filter.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : TcpFilter������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "tcp_filter.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ȷ�������Ƿ�ƥ���������ֻҪ��һ����ƥ��ͷ���false��
 * ��  ��: [in] conn_id ���ӱ�ʶ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] ip_regions �������ļ�����������IP��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
TcpIpFilter::TcpIpFilter(const IpRegionVec& ip_regions)
	: ip_regions_(ip_regions)
{
	TMAS_ASSERT(!ip_regions_.empty());
}

/*------------------------------------------------------------------------------
 * ��  ��: ȷ�������Ƿ�ƥ���������Դ/Ŀ��IP��ַֻҪ��һ��ƥ���OK��
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] port_region �������ļ����������Ķ˿ڶ�
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
TcpPortFilter::TcpPortFilter(const PortRegionVec& port_regions)
	: port_regions_(port_regions)
{
	TMAS_ASSERT(!port_regions_.empty());
}

/*------------------------------------------------------------------------------
 * ��  ��: ȷ�������Ƿ�ƥ���������Դ/Ŀ�Ķ˿�ֻҪ��һ�����þ�OK��
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: �Ƿ�ƥ�������
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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