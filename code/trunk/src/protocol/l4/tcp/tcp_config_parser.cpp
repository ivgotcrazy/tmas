/*#############################################################################
 * �ļ���   : tcp_config_parser.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : TcpConfigParser��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_config_parser.hpp"
#include "tmas_cfg.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ����TCP�����ļ�
 * ��  ��: [out] tcp_domains TCP����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����һ������������
 * ��  ��: [in] config_domain ������
 *         [out] tcp_domains TCP����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���������������
 * ��  ��: [in] entry_line �����������
 *         [out] tcp_domain TCP������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ������������������
 * ��  ��: [in] config_lines ������
 *         [out] tcp_domain TCP������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����������
 * ��  ��: [in] filter �������ַ���
 *         [out] tcp_domain TCP������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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