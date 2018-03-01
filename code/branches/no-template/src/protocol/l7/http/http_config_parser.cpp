/*##############################################################################
 * �ļ���   : uri_config_parser.cpp
 * ������   : rosan 
 * ����ʱ�� : 2014.02.21
 * �ļ����� : UriConfigParser���ʵ���ļ�  
 * ��Ȩ���� : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#include "http_config_parser.hpp"
#include <glog/logging.h>
#include <fstream>
#include <boost/regex.hpp>

namespace BroadInter
{
/*------------------------------------------------------------------------------
 * ��  ��: ����domain-uri�����ļ�
 * ��  ��: [in] file  �����ļ����ļ���
 *         [out] ok  ���������ļ��Ƿ�ɹ�����ѡ����
 *         [out] error_line ���������ļ�ʧ�ܵĴ����кţ���ѡ����
 *         [out] error_description ���������ļ�ʧ�ܵĴ�����������ѡ����
 * ����ֵ: �����ɹ����domain-uri�б�
 * ��  ��:
 *   ʱ�� 2014.02.21
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
bool HttpConfigParser::Parse(const char* file, SiteParamsMap& host_map, SiteParamsMap& uri_map)
{
	LOG(INFO) << "Start parse http conf";

	bool ret = true;
    static const char* const kLeftBraces = "{";  // �������
    static const char* const kRightBraces = "}";  // �Ҵ�����

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
        // ��������
        std::string domain;
        std::getline(ifs, domain);
        ++line;

        if (domain.empty()) continue;
		
		//���ȷֱ�Host����Uri
		//ƥ��������
		//ƥ���������ֶ�
		//ƥ��������

		std::string key;
		std::string value;
		RegularMacth(domain, key, value);

		if (key != "host" && key != "uri") continue;

        // �����������
        std::string left_braces;
        std::getline(ifs, left_braces);
        ++line;

        if (left_braces != kLeftBraces)  // �����������ʧ��
        {
            break;
        }

        //����ĳ�������µ������ڲ����б�
		ParamsMap params_map;
		std::string path;
		bool inner_process_result = true;
        while (!ifs.eof() && ifs.good())
        {
            std::getline(ifs, path);
            ++line;

            if (path == kRightBraces)  // �����Ҵ�����
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
		
        if (path != kRightBraces)  // �����Ƿ�����
        {
            LOG(ERROR) << "Right braces is missing";
			ret = false;
            break;
        }

 		//������һ����󣬽����������²���
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
    boost::smatch match;  // ������ʽƥ��Ľ��
    boost::regex reg("\\s*(\\w+)\\s*([a-zA-Z0-9._/*]+)\\s*.*");  //ƥ�� key value

	bool match_result = false;
    if (boost::regex_match(origin, match, reg) && match.size() == 3)  // ������ʽƥ��
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
	
	boost::smatch match;  // ������ʽƥ��Ľ��
    boost::regex reg("\\s*(\\w+)\\s*([a-zA-Z0-9._/*]+)\\s*.*");  //ƥ�� key value

	bool match_result = false;
    if (boost::regex_match(origin, match, reg) && match.size() == 3)  // ������ʽƥ��
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


