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

namespace BroadInter
{
/*------------------------------------------------------------------------------
 * ��  ��: ����Host����
 * ��  ��: [in] entry_value Host������ʽ
 *         [in] config_lines �����ڲ�����
 *         [out] host_configs �������ü���
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseHostConfig(const string& entry_value, 
	                                   const ConfigLineVec& config_lines,
									   HostConfigVec& host_configs)
{
	HostConfig host_config;

	host_config.host = entry_value;

	for (const string& config_line : config_lines)
	{
		string config_key, config_value;
		ConfigItemMatch(config_line, config_key, config_value);

		if (config_key == "delay_monitor")
		{
			if (config_value == "on")
			{
				host_config.delay_monitor = true;
			}
			else if (config_value == "off")
			{
				host_config.delay_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid delay_monitor : " << config_value;
				return false;
			}
		}
		else if (config_key == "min_delay")
		{
			try 
			{
				host_config.min_delay
					= boost::lexical_cast<uint32>(config_value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid min-delay value : " << config_value;
				return false;
			}
		}
		else if (config_key == "speed_monitor")
		{
			if (config_value == "on")
			{
				host_config.speed_monitor = true;
			}
			else if (config_value == "off")
			{
				host_config.speed_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid speed_monitor : " << config_value;
				return false;
			}
		}
		else if (config_key == "min_speed")
		{
			try 
			{
				host_config.min_speed
					= boost::lexical_cast<uint32>(config_value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid min-speed value : " << config_value;
				return false;
			}
		}
		else if (config_key == "stat_interval")
		{
			try 
			{
				host_config.stat_interval
					= boost::lexical_cast<uint32>(config_value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid stat-interval value : " 
					       << config_value;
				return false;
			}
		}
		else if (config_key == "src_ip")
		{	
			if (!ParseIpItem(config_value, host_config.src_ips))
			{
				LOG(ERROR) << "Parse src_ip failed";
				return false;
			}
		}
		else if (config_key == "dst_ip")
		{
			if (!ParseIpItem(config_value, host_config.dst_ips))
			{
				LOG(ERROR) << "Parse dst_ip failed";
				return false;
			}
		}
		else if (config_key == "uri")
		{
			host_config.uris.push_back(config_value);
		}
		else
		{
			LOG(ERROR) << "Invalid config key : " << config_key;
			return false;
		}
	}

	host_configs.push_back(host_config);

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����Uri����
 * ��  ��: [in] entry_value Uri������ʽ
 *         [in] config_lines �����ڲ�����
 *         [out] uri_config URI���ü���
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseUriConfig(const string& entry_value, 
	                                  const ConfigLineVec& config_lines,
									  UriConfigVec& uri_configs)
{
	UriConfig uri_config;

	uri_config.uri = entry_value;

	for (const string& config_line : config_lines)
	{
		string config_key, config_value;
		ConfigItemMatch(config_line, config_key, config_value);

		if (config_key == "delay_monitor")
		{
			if (config_value == "on")
			{
				uri_config.delay_monitor = true;
			}
			else if (config_value == "off")
			{
				uri_config.delay_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid delay_monitor : " << config_value;
				return false;
			}
		}
		else if (config_key == "min_delay")
		{
			try 
			{
				uri_config.min_delay
					= boost::lexical_cast<uint32>(config_value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid min-delay value : " << config_value;
				return false;
			}
		}
		else if (config_key == "speed_monitor")
		{
			if (config_value == "on")
			{
				uri_config.speed_monitor = true;
			}
			else if (config_value == "off")
			{
				uri_config.speed_monitor = false;
			}
			else
			{
				LOG(ERROR) << "Invalid speed_monitor : " << config_value;
				return false;
			}
		}
		else if (config_key == "min_speed")
		{
			try 
			{
				uri_config.min_speed
					= boost::lexical_cast<uint32>(config_value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid min-speed value : " << config_value;
				return false;
			}
		}
		else if (config_key == "src_ip")
		{	
			if (!ParseIpItem(config_value, uri_config.src_ips))
			{
				LOG(ERROR) << "Parse src_ip failed";
				return false;
			}
		}
		else if (config_key == "dst_ip")
		{
			if (!ParseIpItem(config_value, uri_config.dst_ips))
			{
				LOG(ERROR) << "Parse dst_ip failed";
				return false;
			}
		}
		else if (config_key == "stat_interval")
		{
			try 
			{
				uri_config.stat_interval 
					= boost::lexical_cast<uint32>(config_value);
			}
			catch (...)
			{
				LOG(ERROR) << "Invalid stat-interval : " << config_value;
				return false;
			}
		}
		else
		{
			LOG(ERROR) << "Invalid config key : " << config_key;
			return false;
		}
	}

	uri_configs.push_back(uri_config);

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����Entry
 * ��  ��: [in] entry_line entry������
 *         [in] config_lines entry�ڲ�����
 *         [out] host_configs �������ü���
 *         [out] uri_configs URI���ü���
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseConfigEntry(const string& entry_line, 
	                                    const ConfigLineVec& config_lines,
										HostConfigVec& host_configs,
										UriConfigVec& uri_configs)
{
	std::string entry_type, entry_value;
	ConfigItemMatch(entry_line, entry_type, entry_value);

	if (entry_type == "host")
	{
		if (IfHostEntryExist(entry_value, host_configs))
		{
			LOG(ERROR) << "Repeated host entry | " << entry_value;
			return false;
		}

		return ParseHostConfig(entry_value, config_lines, host_configs);
	}
	else if (entry_type == "uri")
	{
		if (IfUriEntryExist(entry_value, uri_configs))
		{
			LOG(ERROR) << "Repeated uri entry | " << entry_value;
			return false;
		}

		return ParseUriConfig(entry_value, config_lines, uri_configs);
	}
	else
	{
		LOG(ERROR) << "Unrecognized config entry type : " << entry_type;
		return false;
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: [out] host_configs �������ü���
 *         [out] uri_configs URI���ü���
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseConfig(HostConfigVec& host_configs, UriConfigVec& uri_configs)
{
	static const char* const kLeftBraces = "{";  // �������
	static const char* const kRightBraces = "}";  // �Ҵ�����

	std::ifstream ifs(HTTP_CONFIG_PATH);
	if (!ifs.is_open())
	{
		DLOG(ERROR) << "Fail to open config file : " << HTTP_CONFIG_PATH;
		return false;
	}

	while (!ifs.eof() && ifs.good())
	{
		//--- �������������

		std::string entry_line;
		std::getline(ifs, entry_line);
		boost::trim(entry_line);

		if (entry_line.empty()) continue;	// ����
		if (entry_line[0] == '#') continue; // ע��

		//--- �����������

		std::string left_braces;
		std::getline(ifs, left_braces);
		boost::trim(left_braces);

		if (left_braces != kLeftBraces)  // �����������ʧ��
		{
			LOG(ERROR) << "Fail to find left brace";
			return false;
		}

		//--- ���������ڲ���

		bool find_right_brace = false;
		std::vector<string> entry_config;
		while (!ifs.eof() && ifs.good())
		{
			std::string config_line;
			std::getline(ifs, config_line);
			boost::trim(config_line);

			if (config_line.empty()) continue;	 // ����
			if (config_line[0] == '#') continue; // ע��

			if (config_line == kRightBraces)
			{
				find_right_brace = true;
				break;
			}

			entry_config.push_back(config_line);
		}

		if (!find_right_brace)
		{
			LOG(ERROR) << "Can not find right brace";
			return false;
		}

		if (!ParseConfigEntry(entry_line, entry_config, 
			                  host_configs, uri_configs))
		{
			return false;
		}
	}

	DLOG(INFO) << "Parse http config successfully";

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ������У��ͽ���
 * ��  ��: [in] origin ԭʼ��������
 *         [out] key ����������key
 *         [out] value ����������value
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ConfigItemMatch(const string& origin, string& key, string& value)
{
    static boost::regex reg("\\s*(\\w+)\\s*([\a-zA-Z0-9._/*]+)\\s*.*");  //ƥ�� key value

	boost::smatch match;  // ������ʽƥ��Ľ��
    if (boost::regex_match(origin, match, reg) && match.size() == 3)
    {
		key = match[1];
		value = match[2];
		return true;
    }

    return false;

}

/*------------------------------------------------------------------------------
 * ��  ��: �ж�Host�����Ƿ��ظ�
 * ��  ��: [in] entry Host�����ַ���
 *         [in] host_configs �Ѿ������õ�Host����
 * ����ֵ: �Ƿ��Ѿ�����
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::IfHostEntryExist(const string& entry, const HostConfigVec& host_configs)
{
	for (const HostConfig& host_config : host_configs)
	{
		if (host_config.host == entry)
		{
			return true;
		}
	}

	return false;
}

/*------------------------------------------------------------------------------
 * ��  ��: �ж�URI�����Ƿ��ظ�
 * ��  ��: [in] entry URI�����ַ���
 *         [in] uri_configs �Ѿ������õ�URI����
 * ����ֵ: �Ƿ��Ѿ�����
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::IfUriEntryExist(const string& entry, const UriConfigVec& uri_configs)
{
	for (const UriConfig& uri_config : uri_configs)
	{
		if (uri_config.uri == entry)
		{
			return true;
		}
	}

	return false;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����Ip��ַ�����ֶ�
 * ��  ��: [in] config_value �����ֶ�
 *         [in] ip_addr_vec �洢��ַ����
 * ����ֵ: �Ƿ��Ѿ�����
 * ��  ��:
 *   ʱ�� 2014��04��017��
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseIpItem(const string config_value, IpAddrVec& ip_addr_vec)
{
	uint32 cur = 0;

	//�α��ƶ����ǿ�λ��
	while (config_value[cur] == ' ')
	{
		cur++;
		continue;
	}

	if (cur > config_value.size()) return false;

	uint32 begin = 0;
	while (cur <= config_value.size())
	{
		while (config_value[cur] != ' ' && cur < config_value.size())
		{
			cur++;
			continue;
		}
	
		string ip_item = config_value.substr(begin, cur - begin);
		DLOG(INFO) << "Ip item: " << ip_item;

		//�ж�Ip��ַ��Ϣ�Ƿ���Ϲ淶
		if (!VerifyIpAddr(ip_item))
		{
			LOG(ERROR) << "Ip addr don't match the standard | value:" << ip_item;
			return false;
		}

		//��ǰ����ַ��û�г�ͻ��û�оͽ��в���
		if (!AddIpAddr(ip_item, ip_addr_vec))
		{
			LOG(ERROR) << "Ip addr conflict";
			return false;
		}
		
		cur++;
		begin = cur;
	}

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: �ж�Ip��ַ���Ƿ���ϸ�ʽҪ��
 * ��  ��: [in] ip_addr Ip��ַ��
 * ����ֵ: �Ƿ����Ҫ��
 * ��  ��:
 *   ʱ�� 2014��04��14��
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::VerifyIpAddr(const string ip_addr)
{
	//��֤��׼: 1.ƥ��ģʽ�ж�  2.�ĸ�����ֶ�. 
	if (ip_addr.find('/') != string::npos)
	{
		string::size_type pos = 0;
		string::size_type found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;
		found = ip_addr.find('/', pos);
		if (found == string::npos) return false;
		
		//DLOG(INFO) << "substr: " << ip_addr.substr(found + 1);

		uint64 net_mask = 0;
		try 
		{
			net_mask = boost::lexical_cast<uint32>(ip_addr.substr(found + 1));
		}
		catch (...)
		{
			LOG(ERROR) << "Parse net_mask failed, addr: " << ip_addr;
			return false;
		}
		
		if (net_mask > 32 || net_mask < 0)
		{
			LOG(ERROR) << "netmask is invalid";
			return false;
		}
	}
	else if (ip_addr.find('-') != string::npos)
	{
		string::size_type pos = 0;
		string::size_type found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;	
		found = ip_addr.find('-', pos);
		if (found == string::npos) return false;
		
		string::size_type split_pos = found;
		//LOG(INFO) << "Start Addr: " << ip_addr.substr(0, split_pos).c_str();
		uint64 start_addr = ntohl(inet_addr(ip_addr.substr(0, split_pos).c_str()));

		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		pos = found;
		found = ip_addr.find('.', pos);
		if (found == string::npos) return false;
		
		//LOG(INFO) << "End Addr: " << ip_addr.substr(split_pos + 1).c_str();
		uint64 end_addr = ntohl(inet_addr(ip_addr.substr(split_pos + 1).c_str()));

		if (end_addr < start_addr)
		{
			LOG(ERROR) << "End addr is bigger than start addr.";
		}
	}
	else
	{
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ���Ip��ַ
 * ��  ��: [in] ip_item Ip��ַ��
 *         [in] ip_addr_vec Ip��ַ�洢����
 * ����ֵ: ����Ƿ�ɹ�
 * ��  ��:
 *   ʱ�� 2014��04��14��
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::AddIpAddr(const string ip_item, IpAddrVec& ip_addr_vec)
{
	uint32 begin_addr = 0;
	uint32 end_addr = 0;
	string::size_type found = ip_item.find('/');
	if (found != string::npos)
	{
		uint32 address = ntohl(inet_addr(ip_item.substr(0, found).c_str()));
		uint32 net_mask = boost::lexical_cast<uint32>(ip_item.substr(found + 1));

		TMAS_ASSERT(net_mask < 32);

		uint32 offset = 0xFFFFFFFF >> net_mask;
		if ((address & offset) != 0)
		{
			begin_addr = end_addr = address;
		}
		else
		{
			begin_addr = address;
			end_addr = address | offset;
		}
		//DLOG(INFO) << "begin: " << begin_addr << " end: " << end_addr;
	}
	else
	{
		string::size_type pos = ip_item.find('-');
	    begin_addr =ntohl(inet_addr(ip_item.substr(0, pos).c_str()));
	    end_addr = ntohl(inet_addr(ip_item.substr(0, pos).c_str()));
	}

	IpAddrRegion region;
	region.start_addr = begin_addr;
	region.end_addr = end_addr;

	if (!ip_addr_vec.empty())
	{
		for (auto addr_region : ip_addr_vec)
		{
			if (IsIpAddrConflict(region.start_addr, region.end_addr, 
									addr_region.start_addr, addr_region.end_addr))
			{	
				LOG(ERROR) << "Ip Addr Conflict |orign addr ";
				return false;
			}
		}
	}

	ip_addr_vec.push_back(region);
	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: �ж�Ip��ַ�Ƿ��ͻ
 * ��  ��: [in] start ��ʼ��ַ
 *         [in] end ������ַ
 *         [in] cmp_start �ȽϿ�ʼ��ַ
 *         [in] cmp_end �ȽϽ�����ַ
 * ����ֵ: �Ƿ��е�ַ��ͻ, ��ͻ������
 * ��  ��:
 *   ʱ�� 2014��04��16��
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::IsIpAddrConflict(const uint32 start, const uint32 end
											,const uint32 cmp_start, const uint32 cmp_end)
{
	if (start == end)
	{
		if (start > cmp_start && start < cmp_end)
		{
			return true;
		}
	}
	else
	{
		if (start > cmp_end || end < cmp_start)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	
	return false;
}

}  // namespace BroadInter


