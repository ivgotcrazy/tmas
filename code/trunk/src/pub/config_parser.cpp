/*#############################################################################
 * �ļ���   : config_parser.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��09��
 * �ļ����� : ���ý�������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "config_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ������У��ͽ���
 * ��  ��: [in] config_file �����ļ�
 *         [out] config_domains ������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ConfigParser::ParseConfig(const string& config_file, ConfigDomainVec& config_domains)
{
	static const char* const kLeftBraces = "{";  // �������
	static const char* const kRightBraces = "}";  // �Ҵ�����

	std::ifstream ifs(config_file);
	if (!ifs.is_open())
	{
		DLOG(ERROR) << "Fail to open config file | " << config_file;
		return false;
	}

	ConfigDomain config_domain;
	while (!ifs.eof() && ifs.good())
	{
		//--- �������������

		std::getline(ifs, config_domain.entry_line);
		boost::trim(config_domain.entry_line);

		if (config_domain.entry_line.empty()) 
		{
			continue;	// ����
		}

		if (config_domain.entry_line[0] == '#') 
		{
			continue;	// ע��
		}

		//--- �����������

		std::string left_braces;
		std::getline(ifs, left_braces);
		boost::trim(left_braces);

		if (left_braces != kLeftBraces)  // �����������ʧ��
		{
			DLOG(ERROR) << "Fail to find left brace";
			return false;
		}

		//--- ���������ڲ���

		bool find_right_brace = false;
		while (!ifs.eof() && ifs.good())
		{
			std::string config_line;
			std::getline(ifs, config_line);
			boost::trim(config_line);

			if (config_line.empty()) 
			{
				continue;	// ����
			}

			if (config_line[0] == '#') 
			{
				continue;	// ע��
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
 * ��  ��: ������У��ͽ���
 * ��  ��: [in] item ������
 *         [out] key ����������key
 *         [out] value ����������value
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ConfigParser::ConfigItemMatch(const string& item, string& key, string& value)
{
	// ƥ���ʽ��"key value"
    static boost::regex reg("\\s*([\\w-]+)\\s*([\a-zA-Z0-9._/*\\s]+)\\s*$");  

	boost::smatch match;  // ������ʽƥ��Ľ��
    if (boost::regex_match(item, match, reg) && match.size() == 3)
    {
		key = match[1];
		value = match[2];
		return true;
    }

    return false;
}

}