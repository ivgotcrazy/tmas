/*##############################################################################
 * 文件名   : http_config_parser.hpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年3月29日
 * 文件描述 : HttpConfigParser类的声明文件
 * 版权声明 : Copyright(c)2014 BroadInter.All rights reserved.
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
 * 描  述: HTTP配置域
 * 作  者: teck_zhou
 * 时  间: 2014年05月09日
 ******************************************************************************/
struct HttpConfigDomain
{
	HttpConfigDomain()
		: resp_delay_monitor(false)
		, dl_speed_monitor(false)
		, access_monitor(false)
		, resp_delay_min(0)
		, dl_speed_max(0) {}

	bool resp_delay_monitor;	// 响应时延监视开关
	bool dl_speed_monitor;		// 下载速度监视开关
	bool access_monitor;		// 访问监视开关

	uint32 resp_delay_min;		// 大于此时延才记录
	uint32 dl_speed_max;		// 小于此速度才记录

	IpRegionVec ip_regions;		// 匹配的IP地址段
	PortRegionVec port_regions;	// 匹配的端口段

	string host_regex;			// 域名匹配
	string uri_regex;			// URI匹配
};

typedef std::vector<HttpConfigDomain> HttpConfigDomainVec;

/*******************************************************************************
 * 描  述: HTTP配置文件解析器
 * 作  者: teck_zhou
 * 时  间: 2014年05月09日
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
