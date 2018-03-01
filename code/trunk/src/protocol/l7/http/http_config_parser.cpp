/*##############################################################################
 * �ļ���   : uri_config_parser.cpp
 * ������   : rosan 
 * ����ʱ�� : 2014.02.21
 * �ļ����� : UriConfigParser���ʵ���ļ�  
 * ��Ȩ���� : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/

#include <glog/logging.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "http_config_parser.hpp"
#include "tmas_cfg.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ����������
 * ��  ��: [in] filter �������������ַ���
 *         [out] config_domain ������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseFilter(const string& filter, HttpConfigDomain& http_domain)
{
	string key, value;
	if (!ConfigParser::ConfigItemMatch(filter, key, value))
	{
		LOG(ERROR) << "Invalid config item | " << filter;
		return false;
	}

	if (key == "host")
	{
		if (!http_domain.host_regex.empty())
		{
			LOG(ERROR) << "More than one host regex configured | " << value;
			return false;
		}

		http_domain.host_regex = value;
	}
	else if (key == "uri")
	{
		if (!http_domain.uri_regex.empty())
		{
			LOG(ERROR) << "More than one uri regex configured | " << value;
			return false;
		}

		http_domain.uri_regex = value;
	}
	else if (key == "ip")
	{
		if (!IpRegionParser::ParseIpRegions(value, http_domain.ip_regions))
		{
			LOG(ERROR) << "Fail to parse ip region | " << value;
			return false;
		}
	}
	else if (key == "port")
	{
		if (!PortRegionParser::ParsePortRegions(value, http_domain.port_regions))
		{
			LOG(ERROR) << "Fail to parse port region | " << value;
			return false;
		}
	}
	else
	{
		LOG(ERROR) << "Invalid filter | " << key;
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ������������У���������а�����������
 * ��  ��: [in] entry_line ���������
 *         [out] config_domain ������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseEntryLine(const string& entry_line, 
	                                  HttpConfigDomain& http_domain)
{
	std::vector<string> filters = SplitStr(entry_line, '|');
	if (filters.empty())
	{
		LOG(ERROR) << "No filter in entry line";
		return false;
	}

	for (const string& filter : filters)
	{
		if (!ParseFilter(filter, http_domain))
		{
			LOG(ERROR) << "Fail to parse filter | " << filter;
			return false;
		}
	}

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: �����������е�������
 * ��  ��: [in] config_lines ������
 *         [out] config_domain ������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseConfigLines(const ConfigLineVec& config_lines, 
	                                    HttpConfigDomain& http_domain)
{
	for (const string& config_line : config_lines)
	{
		string key, value;
		if (!ConfigParser::ConfigItemMatch(config_line, key, value))
		{
			LOG(ERROR) << "Invalid config item | " << config_line;
			return false;
		}

		if (key == "response-delay-monitor")
		{
			if (value == "on")
			{
				http_domain.resp_delay_monitor = true;
			}
			else if (value == "off")
			{
				http_domain.resp_delay_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid response-delay-monitor | " << value;
				return false;
			}
		}
		else if (key == "response-delay-min")
		{
			try 
			{
				http_domain.resp_delay_min
					= boost::lexical_cast<uint32>(value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid response-delay-min | " << value;
				return false;
			}
		}
		else if (key == "download-speed-monitor")
		{
			if (value == "on")
			{
				http_domain.dl_speed_monitor = true;
			}
			else if (value == "off")
			{
				http_domain.dl_speed_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid download-speed-monitor : " << value;
				return false;
			}
		}
		else if (key == "download-speed-max")
		{
			try 
			{
				http_domain.dl_speed_max
					= boost::lexical_cast<uint32>(value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid download-speed-max value : " << value;
				return false;
			}
		}
		else if (key == "access-monitor")
		{
			if (value == "on")
			{
				http_domain.access_monitor = true;
			}
			else if (value == "off")
			{
				http_domain.access_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid access-monitor : " << value;
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
 * ��  ��: ����һ������������
 * ��  ��: [in] config_domain ������
 *         [out] http_domains HTTP����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseConfigDomain(const ConfigDomain& config_domain, 
										HttpConfigDomainVec& http_domains)
{
	HttpConfigDomain http_domain;
	
	if (!ParseEntryLine(config_domain.entry_line, http_domain))
	{
		return false;
	}

	if (!ParseConfigLines(config_domain.config_lines, http_domain))
	{
		return false;
	}
	
	http_domains.push_back(http_domain);

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����HTTP����
 * ��  ��: [out] config_domains ����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseHttpConfig(HttpConfigDomainVec& http_domains)
{
	ConfigDomainVec config_domains;
	if (!ConfigParser::ParseConfig(HTTP_CONFIG_PATH, config_domains))
	{
		LOG(ERROR) << "Fail to parse config file | " << HTTP_CONFIG_PATH;
		return false;
	}

	for (const ConfigDomain& config_domain : config_domains)
	{
		if (!ParseConfigDomain(config_domain, http_domains))
		{
			LOG(ERROR) << "Fail to parse config domain";
			return false;
		}
	}

	return true;
}

}  // namespace BroadInter


