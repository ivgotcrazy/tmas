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

namespace BroadInter
{

using std::string;

//typedef std::vector<std::string> IpAddrVec;

/*******************************************************************************
 * 描  述: 起始结束Ip地址对信息
 * 作  者: tom_liu
 * 时  间: 2014年04月17日
 ******************************************************************************/
struct IpAddrRegion
{
	IpAddrRegion() : start_addr(0), end_addr(0){}
	
	uint32 start_addr;
	uint32 end_addr;
};

typedef std::vector<IpAddrRegion> IpAddrVec;

/*******************************************************************************
 * 描  述: HOST配置
 * 作  者: teck_zhou
 * 时  间: 2014年03月29日
 ******************************************************************************/
struct HostConfig
{
	HostConfig() : min_delay(0), min_speed(0), stat_interval(0) {}

	string host;
	
	bool delay_monitor;		// 访问时延监测开关
	uint32 min_delay;		// 大于此时延才记录

	bool speed_monitor;		// 下载速度监测开关
	uint32 min_speed;		// 大于此速度才记录

	IpAddrVec src_ips;
	IpAddrVec dst_ips;

	uint32 stat_interval;	// 记录时间间隔

	std::vector<string> uris;
};

/*******************************************************************************
 * 描  述: URI配置
 * 作  者: teck_zhou
 * 时  间: 2014年03月29日
 ******************************************************************************/
struct UriConfig
{
	UriConfig() : min_delay(0), min_speed(0), stat_interval(0) {}

	string uri;

	bool delay_monitor;		// 访问时延监测开关
	uint32 min_delay;		// 大于此时延才记录

	bool speed_monitor;		// 下载速度监测开关
	uint32 min_speed;		// 大于此速度才记录

	IpAddrVec src_ips;
	IpAddrVec dst_ips;

	uint32 stat_interval;	// 记录时间间隔
};

typedef std::vector<HostConfig> HostConfigVec;
typedef std::vector<UriConfig> UriConfigVec;

/*******************************************************************************
 * 描  述: HTTP配置文件解析器
 * 作  者: teck_zhou
 * 时  间: 2014年03月29日
 ******************************************************************************/
class HttpConfigParser
{
public:
	static bool ParseConfig(HostConfigVec& host_configs, UriConfigVec& uri_configs);

private:
	typedef std::vector<string> ConfigLineVec;

	static bool ConfigItemMatch(const string& origin, string& key, string& value);

	static bool ParseConfigEntry(const string& entry_line, const ConfigLineVec& config_lines, 
		HostConfigVec& host_config, UriConfigVec& uri_configs);

	static bool ParseUriConfig(const string& entry_value, 
		const ConfigLineVec& config_lines, UriConfigVec& uri_configs);

	static bool ParseHostConfig(const string& entry_value,
		const ConfigLineVec& config_lines, HostConfigVec& host_configs);

	static bool IfHostEntryExist(const string& entry, const HostConfigVec& host_configs);

	static bool IfUriEntryExist(const string& entry, const UriConfigVec& uri_configs);

	static bool ParseIpItem(const string config_value, IpAddrVec& ip_addr_vec);

	static bool VerifyIpAddr(const string ip_addr);

	static bool AddIpAddr(const string ip_item, IpAddrVec& ip_addr_vec);

	static bool IsIpAddrConflict(const uint32 start, const uint32 end
											,const uint32 cmp_start, const uint32 cmp_end);
};

}  // namespace BroadInter

#endif  // BROADINTER_URI_CONFIG_PARSER
