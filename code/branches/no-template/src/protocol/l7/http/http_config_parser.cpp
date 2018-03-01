/*##############################################################################
 * 文件名   : uri_config_parser.cpp
 * 创建人   : rosan 
 * 创建时间 : 2014.02.21
 * 文件描述 : UriConfigParser类的实现文件  
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#include "http_config_parser.hpp"
#include <glog/logging.h>
#include <fstream>
#include <boost/regex.hpp>

namespace BroadInter
{
/*------------------------------------------------------------------------------
 * 描  述: 解析domain-uri配置文件
 * 参  数: [in] file  配置文件的文件名
 *         [out] ok  解析配置文件是否成功，可选参数
 *         [out] error_line 解析配置文件失败的错误行号，可选参数
 *         [out] error_description 解析配置文件失败的错误描述，可选参数
 * 返回值: 解析成功后的domain-uri列表
 * 修  改:
 *   时间 2014.02.21
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
bool HttpConfigParser::Parse(const char* file, SiteParamsMap& host_map, SiteParamsMap& uri_map)
{
	LOG(INFO) << "Start parse http conf";

	bool ret = true;
    static const char* const kLeftBraces = "{";  // 左大括号
    static const char* const kRightBraces = "}";  // 右大括号

    std::ifstream ifs(file);
    if (!ifs.is_open())
    {
        LOG(INFO) << "Fail to open uri config file : " << file;
        return false;
    }

    size_t line = 0;
    std::string description;

    while (!ifs.eof() && ifs.good())
    {
        // 解析域名
        std::string domain;
        std::getline(ifs, domain);
        ++line;

        if (domain.empty()) continue;
		
		//首先分辨Host还是Uri
		//匹配左括号
		//匹配括号内字段
		//匹配右括号

		std::string key;
		std::string value;
		RegularMacth(domain, key, value);

		if (key != "host" && key != "uri") continue;

        // 解析左大括号
        std::string left_braces;
        std::getline(ifs, left_braces);
        ++line;

        if (left_braces != kLeftBraces)  // 解析左大括号失败
        {
            break;
        }

        //解析某个域名下的括号内参数列表
		ParamsMap params_map;
		std::string path;
		bool inner_process_result = true;
        while (!ifs.eof() && ifs.good())
        {
            std::getline(ifs, path);
            ++line;

            if (path == kRightBraces)  // 遇到右大括号
                break;

			std::string item_name;
			std::string item_value;
		
			if (!RegularMacth(path, item_name, item_value))
			{
				LOG(ERROR) << "Analyze the item failed";
				inner_process_result = false;
				break;
			}
			
            params_map.insert(std::make_pair(item_name, item_value)); 
        }

		if (!inner_process_result) break;
		
        if (path != kRightBraces)  // 解析非法结束
        {
            LOG(ERROR) << "Right braces is missing";
			ret = false;
            break;
        }

 		//处理完一个域后，进行容器更新操作
 		if (key == "host")
 		{
        	host_map.insert(std::make_pair(value, params_map));
 		}
		else
		{
			uri_map.insert(std::make_pair(value, params_map));
		}

    }

    return ret;
}

bool HttpConfigParser::RegularMacth(const std::string origin, std::string& key, std::string& value)
{
    boost::smatch match;  // 正则表达式匹配的结果
    boost::regex reg("\\s*(\\w+)\\s*([a-zA-Z0-9._/*]+)\\s*.*");  //匹配 key value

	bool match_result = false;
    if (boost::regex_match(origin, match, reg) && match.size() == 3)  // 正则表达式匹配
    {
		key = match[1];
		value = match[2];
		LOG(INFO) << " key: " << key << " value: " << value;
		match_result = true;
    }

    return match_result;
}

#if 0
bool HttpConfigParser::IsInterestDomain(const std::string  domain)
{
	auto iter = host_site_map_.find(domain);

	if (iter == host_site_map_.end()) return false;

	return true;
}

bool HttpConfigParser::IsInterestUri(const std::string domain, const std::string uri)
{
	
	boost::smatch match;  // 正则表达式匹配的结果
    boost::regex reg("\\s*(\\w+)\\s*([a-zA-Z0-9._/*]+)\\s*.*");  //匹配 key value

	bool match_result = false;
    if (boost::regex_match(origin, match, reg) && match.size() == 3)  // 正则表达式匹配
    {
		key = match[1];
		value = match[2];
		LOG(INFO) << " key: " << key << " value: " << value;
		match_result = true;
    }
}

HttpConfigParser& HttpConfigParser::GetInstance()
{
	static HttpConfigParser instance;
    return instance;
}

#endif
}  // namespace BroadInter


