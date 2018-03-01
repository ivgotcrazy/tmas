/*#############################################################################
 * �ļ���   : config_parser.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��09��
 * �ļ����� : ���ý�������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: ������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
struct ConfigDomain
{
	string entry_line;
	ConfigLineVec config_lines;
};

typedef std::vector<ConfigDomain> ConfigDomainVec;

/******************************************************************************
 * ����: �����ļ�����������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class ConfigParser
{
public:
	static bool ParseConfig(const string& config_file, ConfigDomainVec& config_domains);

	static bool ConfigItemMatch(const string& item, string& key, string& value);
};

}

#endif
