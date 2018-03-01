/*#############################################################################
 * 文件名   : config_parser.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月09日
 * 文件描述 : 配置解析重用
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CONFIG_PARSER
#define BROADINTER_CONFIG_PARSER

#include <vector>
#include <string>

namespace BroadInter
{

using std::string;

typedef std::vector<string> ConfigLineVec;

/******************************************************************************
 * 描述: 配置域
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
struct ConfigDomain
{
	string entry_line;
	ConfigLineVec config_lines;
};

typedef std::vector<ConfigDomain> ConfigDomainVec;

/******************************************************************************
 * 描述: 配置文件公共解析器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class ConfigParser
{
public:
	static bool ParseConfig(const string& config_file, ConfigDomainVec& config_domains);

	static bool ConfigItemMatch(const string& item, string& key, string& value);
};

}

#endif
