/*#############################################################################
 * 文件名   : tcp_config_parser.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : TcpConfigParser类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_config_parser.hpp"
#include "tmas_cfg.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 解析TCP配置文件
 * 参  数: [out] tcp_domains TCP配置域容器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpConfigParser::ParseTcpConfig(TcpConfigDomainVec& tcp_domains)
{
	ConfigDomainVec config_domains;

	if (!ConfigParser::ParseConfig(TCP_CONFIG_PATH, config_domains))
	{
		LOG(ERROR) << "Fail to parse config file | " << TCP_CONFIG_PATH;
		return false;
	}

	for (const ConfigDomain& config_domain : config_domains)
	{
		if (!ParseConfigDomain(config_domain, tcp_domains))
		{
			LOG(ERROR) << "Fail to parse config domain";
			return false;
		}
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析一个完整配置域
 * 参  数: [in] config_domain 配置域
 *         [out] tcp_domains TCP配置域容器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpConfigParser::ParseConfigDomain(const ConfigDomain& config_domain, 
	                                    TcpConfigDomainVec& tcp_domains)
{
	TcpConfigDomain tcp_domain;

	if (!ParseEntryLine(config_domain.entry_line, tcp_domain))
	{
		return false;
	}

	if (!ParseConfigLines(config_domain.config_lines, tcp_domain))
	{
		return false;
	}

	tcp_domains.push_back(tcp_domain);

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析配置域入口行
 * 参  数: [in] entry_line 配置域入口行
 *         [out] tcp_domain TCP配置域
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpConfigParser::ParseEntryLine(const string& entry_line, 
									 TcpConfigDomain& tcp_domain)
{
	std::vector<string> filters = SplitStr(entry_line, '|');
	if (filters.empty())
	{
		LOG(ERROR) << "No filter in entry line";
		return false;
	}

	for (const string& filter : filters)
	{
		if (!ParseFilter(filter, tcp_domain))
		{
			LOG(ERROR) << "Fail to parse filter | " << filter;
			return false;
		}
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析配置域内配置项
 * 参  数: [in] config_lines 配置项
 *         [out] tcp_domain TCP配置域
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpConfigParser::ParseConfigLines(const ConfigLineVec& config_lines, 
	                                   TcpConfigDomain& tcp_domain)
{
	for (const string& config_line : config_lines)
	{
		string key, value;
		if (!ConfigParser::ConfigItemMatch(config_line, key, value))
		{
			LOG(ERROR) << "Invalid config item | " << config_line;
			return false;
		}

		if (key == "handshake-delay-monitor")
		{
			if (value == "on")
			{
				tcp_domain.hs_delay_monitor = true;
			}
			else if (value == "off")
			{
				tcp_domain.hs_delay_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid handshake-delay-monitor | " << value;
				return false;
			}
		}
		else if (key == "handshake-delay-min")
		{
			try 
			{
				tcp_domain.hs_delay_min
					= boost::lexical_cast<uint32>(value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid handshake-delay-min | " << value;
				return false;
			}
		}
		else if (key == "handshake-timeout-monitor")
		{
			if (value == "on")
			{
				tcp_domain.hs_timeout_monitor = true;
			}
			else if (value == "off")
			{
				tcp_domain.hs_timeout_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid handshake-timeout-monitor : " << value;
				return false;
			}
		}
		else if (key == "connection-timeout-monitor")
		{
			if (value == "on")
			{
				tcp_domain.conn_timeout_monitor = true;
			}
			else if (value == "off")
			{
				tcp_domain.conn_timeout_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid connection-timeout-monitor : " << value;
				return false;
			}
		}
		else if (key == "connection-abort-monitor")
		{
			if (value == "on")
			{
				tcp_domain.conn_abort_monitor = true;
			}
			else if (value == "off")
			{
				tcp_domain.conn_abort_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid connection-abort-monitor : " << value;
				return false;
			}
		}
		else
		{
			LOG(ERROR) << "Invalid config key | " << key;
			return false;
		}
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析过滤器
 * 参  数: [in] filter 过滤器字符串
 *         [out] tcp_domain TCP配置域
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpConfigParser::ParseFilter(const string& filter, TcpConfigDomain& tcp_domain)
{
	string key, value;
	if (!ConfigParser::ConfigItemMatch(filter, key, value))
	{
		LOG(ERROR) << "Invalid config item | " << filter;
		return false;
	}

	if (key == "ip")
	{
		if (!IpRegionParser::ParseIpRegions(value, tcp_domain.ip_regions))
		{
			LOG(ERROR) << "Fail to parse ip region | " << value;
			return false;
		}
	}
	else if (key == "port")
	{
		if (!PortRegionParser::ParsePortRegions(value, tcp_domain.port_regions))
		{
			LOG(ERROR) << "Fail to parse port region | " << value;
			return false;
		}
	}
	else
	{
		LOG(ERROR) << "Invalid filter | " << key;
	}

	return true;
}

}