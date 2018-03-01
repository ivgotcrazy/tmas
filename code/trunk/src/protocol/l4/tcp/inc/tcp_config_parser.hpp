/*#############################################################################
 * �ļ���   : tcp_config_parser.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : TcpConfigParser������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONFIG_PARSER
#define BROADINTER_TCP_CONFIG_PARSER

#include <vector>
#include <boost/noncopyable.hpp>

#include "ip_region_parser.hpp"
#include "port_region_parser.hpp"
#include "config_parser.hpp"

namespace BroadInter
{

/******************************************************************************
 * ����: TCP������һ����������������к͡�{}�������ò��֡�
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
struct TcpConfigDomain
{
	TcpConfigDomain()
		: hs_delay_monitor(false)
		, hs_timeout_monitor(false)
		, conn_timeout_monitor(false)
		, conn_abort_monitor(false)
		, hs_delay_min(0) {}

	bool hs_delay_monitor;		// ����ʱ�Ӽ��ӿ���
	bool hs_timeout_monitor;	// ���ֳ�ʱ���ӿ���
	bool conn_timeout_monitor;	// ���ӳ�ʱ���ӿ���
	bool conn_abort_monitor;	// �����쳣�жϼ��ӿ���
	
	uint32 hs_delay_min;		// ʱ�Ӽ�¼��ֵ

	IpRegionVec ip_regions;		// IP��ַ����
	PortRegionVec port_regions;	// �˿ڹ���
};

typedef std::vector<TcpConfigDomain> TcpConfigDomainVec;

/******************************************************************************
 * ����: TCP���ý�����
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class TcpConfigParser : public boost::noncopyable
{
public:
	static bool ParseTcpConfig(TcpConfigDomainVec& tcp_domains);

private:
	static bool ParseConfigDomain(const ConfigDomain& config_domain, TcpConfigDomainVec& tcp_domains);

	static bool ParseEntryLine(const string& entry_line, TcpConfigDomain& tcp_domain);

	static bool ParseConfigLines(const ConfigLineVec& config_lines, TcpConfigDomain& tcp_domain);

	static bool ParseFilter(const string& filter, TcpConfigDomain& tcp_domain);

};

}

#endif