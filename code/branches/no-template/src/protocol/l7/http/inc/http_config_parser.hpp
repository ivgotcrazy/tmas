/*##############################################################################
 * 文件名   : uri_config_parser.hpp
 * 创建人   : rosan 
 * 创建时间 : 2014.02.21
 * 文件描述 : UriConfigParser类的声明文件
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#ifndef BROADINTER_URI_CONFIG_PARSER
#define BROADINTER_URI_CONFIG_PARSER

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace BroadInter
{

typedef boost::unordered_map<std::string, std::string> ParamsMap;

typedef boost::unordered_map<std::string, ParamsMap> SiteParamsMap;

/*******************************************************************************
 * 描  述: 域名对应的uri配置文件读取类
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
class HttpConfigParser
{
public:
    bool Parse(const char* file, SiteParamsMap& host_map, SiteParamsMap& uri_map);
	bool RegularMacth(const std::string origin, std::string& key, std::string& value);

private:
	SiteParamsMap host_site_map_;
	SiteParamsMap uri_site_map_;
};

}  // namespace BroadInter

#endif  // BROADINTER_URI_CONFIG_PARSER
