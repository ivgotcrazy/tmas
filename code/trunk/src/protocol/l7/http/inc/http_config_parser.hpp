/*##############################################################################
 * �ļ���   : http_config_parser.hpp
 * ������   : teck_zhou
 * ����ʱ�� : 2014��3��29��
 * �ļ����� : HttpConfigParser��������ļ�
 * ��Ȩ���� : Copyright(c)2014 BroadInter.All rights reserved.
 * ###########################################################################*/

#ifndef BROADINTER_HTTP_CONFIG_PARSER
#define BROADINTER_HTTP_CONFIG_PARSER

#include <string>
#include <vector>

#include "tmas_typedef.hpp"
#include "ip_region_parser.hpp"
#include "port_region_parser.hpp"
#include "config_parser.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * ��  ��: HTTP������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��09��
 ******************************************************************************/
struct HttpConfigDomain
{
	HttpConfigDomain()
		: resp_delay_monitor(false)
		, dl_speed_monitor(false)
		, access_monitor(false)
		, resp_delay_min(0)
		, dl_speed_max(0) {}

	bool resp_delay_monitor;	// ��Ӧʱ�Ӽ��ӿ���
	bool dl_speed_monitor;		// �����ٶȼ��ӿ���
	bool access_monitor;		// ���ʼ��ӿ���

	uint32 resp_delay_min;		// ���ڴ�ʱ�Ӳż�¼
	uint32 dl_speed_max;		// С�ڴ��ٶȲż�¼

	IpRegionVec ip_regions;		// ƥ���IP��ַ��
	PortRegionVec port_regions;	// ƥ��Ķ˿ڶ�

	string host_regex;			// ����ƥ��
	string uri_regex;			// URIƥ��
};

typedef std::vector<HttpConfigDomain> HttpConfigDomainVec;

/*******************************************************************************
 * ��  ��: HTTP�����ļ�������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��09��
 ******************************************************************************/
class HttpConfigParser
{
public:
	static bool ParseHttpConfig(HttpConfigDomainVec& http_domains);

private:
	static bool ParseConfigDomain(
		const ConfigDomain& config_domain, HttpConfigDomainVec& http_domains);

	static bool ParseEntryLine(
		const string& entry_line, HttpConfigDomain& http_domain);

	static bool ParseConfigLines(
		const ConfigLineVec& config_lines, HttpConfigDomain& http_domain);

	static bool ParseFilter(const string& filter, HttpConfigDomain& http_domain);
};

}  // namespace BroadInter

#endif  // BROADINTER_URI_CONFIG_PARSER
