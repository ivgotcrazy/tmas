/*##############################################################################
 * 文件名   : uri_config_parser.cpp
 * 创建人   : rosan 
 * 创建时间 : 2014.02.21
 * 文件描述 : UriConfigParser类的实现文件  
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
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
 * 描  述: 解析Host配置
 * 参  数: [in] entry_value Host正则表达式
 *         [in] config_lines 域名内部配置
 *         [out] host_configs 域名配置集合
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 解析Uri配置
 * 参  数: [in] entry_value Uri正则表达式
 *         [in] config_lines 域名内部配置
 *         [out] uri_config URI配置集合
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 解析Entry
 * 参  数: [in] entry_line entry行配置
 *         [in] config_lines entry内部配置
 *         [out] host_configs 域名配置集合
 *         [out] uri_configs URI配置集合
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 解析配置
 * 参  数: [out] host_configs 域名配置集合
 *         [out] uri_configs URI配置集合
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseConfig(HostConfigVec& host_configs, UriConfigVec& uri_configs)
{
	static const char* const kLeftBraces = "{";  // 左大括号
	static const char* const kRightBraces = "}";  // 右大括号

	std::ifstream ifs(HTTP_CONFIG_PATH);
	if (!ifs.is_open())
	{
		DLOG(ERROR) << "Fail to open config file : " << HTTP_CONFIG_PATH;
		return false;
	}

	while (!ifs.eof() && ifs.good())
	{
		//--- 解析配置入口行

		std::string entry_line;
		std::getline(ifs, entry_line);
		boost::trim(entry_line);

		if (entry_line.empty()) continue;	// 空行
		if (entry_line[0] == '#') continue; // 注释

		//--- 解析左大括号

		std::string left_braces;
		std::getline(ifs, left_braces);
		boost::trim(left_braces);

		if (left_braces != kLeftBraces)  // 解析左大括号失败
		{
			LOG(ERROR) << "Fail to find left brace";
			return false;
		}

		//--- 解析括号内参数

		bool find_right_brace = false;
		std::vector<string> entry_config;
		while (!ifs.eof() && ifs.good())
		{
			std::string config_line;
			std::getline(ifs, config_line);
			boost::trim(config_line);

			if (config_line.empty()) continue;	 // 空行
			if (config_line[0] == '#') continue; // 注释

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
 * 描  述: 配置项校验和解析
 * 参  数: [in] origin 原始配置内容
 *         [out] key 解析出来的key
 *         [out] value 解析出来的value
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ConfigItemMatch(const string& origin, string& key, string& value)
{
    static boost::regex reg("\\s*(\\w+)\\s*([\a-zA-Z0-9._/*]+)\\s*.*");  //匹配 key value

	boost::smatch match;  // 正则表达式匹配的结果
    if (boost::regex_match(origin, match, reg) && match.size() == 3)
    {
		key = match[1];
		value = match[2];
		return true;
    }

    return false;

}

/*------------------------------------------------------------------------------
 * 描  述: 判断Host配置是否重复
 * 参  数: [in] entry Host配置字符串
 *         [in] host_configs 已经解析好的Host配置
 * 返回值: 是否已经存在
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 判断URI配置是否重复
 * 参  数: [in] entry URI配置字符串
 *         [in] uri_configs 已经解析好的URI配置
 * 返回值: 是否已经存在
 * 修  改:
 *   时间 2014年04月01日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 解析Ip地址配置字段
 * 参  数: [in] config_value 配置字段
 *         [in] ip_addr_vec 存储地址容器
 * 返回值: 是否已经存在
 * 修  改:
 *   时间 2014年04月017日
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::ParseIpItem(const string config_value, IpAddrVec& ip_addr_vec)
{
	uint32 cur = 0;

	//游标移动到非空位置
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

		//判断Ip地址信息是否符合规范
		if (!VerifyIpAddr(ip_item))
		{
			LOG(ERROR) << "Ip addr don't match the standard | value:" << ip_item;
			return false;
		}

		//跟前述地址有没有冲突，没有就进行插入
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
 * 描  述: 判断Ip地址段是否符合格式要求
 * 参  数: [in] ip_addr Ip地址段
 * 返回值: 是否符合要求
 * 修  改:
 *   时间 2014年04月14日
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpConfigParser::VerifyIpAddr(const string ip_addr)
{
	//验证标准: 1.匹配模式判断  2.四个点分字段. 
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
 * 描  述: 添加Ip地址
 * 参  数: [in] ip_item Ip地址段
 *         [in] ip_addr_vec Ip地址存储容器
 * 返回值: 添加是否成功
 * 修  改:
 *   时间 2014年04月14日
 *   作者 tom_liu
 *   描述 创建
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
 * 描  述: 判断Ip地址是否冲突
 * 参  数: [in] start 开始地址
 *         [in] end 结束地址
 *         [in] cmp_start 比较开始地址
 *         [in] cmp_end 比较结束地址
 * 返回值: 是否有地址冲突, 冲突返回真
 * 修  改:
 *   时间 2014年04月16日
 *   作者 tom_liu
 *   描述 创建
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


