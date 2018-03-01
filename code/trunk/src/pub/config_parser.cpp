/*#############################################################################
 * 文件名   : config_parser.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月09日
 * 文件描述 : 配置解析重用
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "config_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 配置项校验和解析
 * 参  数: [in] config_file 配置文件
 *         [out] config_domains 配置域
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool ConfigParser::ParseConfig(const string& config_file, ConfigDomainVec& config_domains)
{
	static const char* const kLeftBraces = "{";  // 左大括号
	static const char* const kRightBraces = "}";  // 右大括号

	std::ifstream ifs(config_file);
	if (!ifs.is_open())
	{
		DLOG(ERROR) << "Fail to open config file | " << config_file;
		return false;
	}

	ConfigDomain config_domain;
	while (!ifs.eof() && ifs.good())
	{
		//--- 解析配置入口行

		std::getline(ifs, config_domain.entry_line);
		boost::trim(config_domain.entry_line);

		if (config_domain.entry_line.empty()) 
		{
			continue;	// 空行
		}

		if (config_domain.entry_line[0] == '#') 
		{
			continue;	// 注释
		}

		//--- 解析左大括号

		std::string left_braces;
		std::getline(ifs, left_braces);
		boost::trim(left_braces);

		if (left_braces != kLeftBraces)  // 解析左大括号失败
		{
			DLOG(ERROR) << "Fail to find left brace";
			return false;
		}

		//--- 解析括号内参数

		bool find_right_brace = false;
		while (!ifs.eof() && ifs.good())
		{
			std::string config_line;
			std::getline(ifs, config_line);
			boost::trim(config_line);

			if (config_line.empty()) 
			{
				continue;	// 空行
			}

			if (config_line[0] == '#') 
			{
				continue;	// 注释
			}

			if (config_line == kRightBraces)
			{
				find_right_brace = true;
				break;
			}

			config_domain.config_lines.push_back(config_line);
		}

		if (!find_right_brace)
		{
			DLOG(ERROR) << "Can not find right brace";
			return false;
		}

		config_domains.push_back(config_domain);
	}

	DLOG(INFO) << "Parse config " << config_file << " successfully";

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 配置项校验和解析
 * 参  数: [in] item 配置项
 *         [out] key 解析出来的key
 *         [out] value 解析出来的value
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月29日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool ConfigParser::ConfigItemMatch(const string& item, string& key, string& value)
{
	// 匹配格式："key value"
    static boost::regex reg("\\s*([\\w-]+)\\s*([\a-zA-Z0-9._/*\\s]+)\\s*$");  

	boost::smatch match;  // 正则表达式匹配的结果
    if (boost::regex_match(item, match, reg) && match.size() == 3)
    {
		key = match[1];
		value = match[2];
		return true;
    }

    return false;
}

}