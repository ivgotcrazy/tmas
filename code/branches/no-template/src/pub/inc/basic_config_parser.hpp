/*#############################################################################
 * �ļ���   : basic_config_parser.hpp
 * ������   : tom_liu	
 * ����ʱ�� : 2013-11-14
 * �ļ����� : BasicConfigParser ͷ�ļ�����
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_BASIC_CONFIG_PARSER
#define BROADINTER_BASIC_CONFIG_PARSER

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace BroadInter
{

namespace po = boost::program_options;
namespace fs = boost::filesystem;

class BasicConfigParser
{
public:
	BasicConfigParser(po::options_description options, const std::string path);
	
	void ParseConfig();

	template<typename T> bool GetValue(const std::string& key, T &t);
	
private:
	po::options_description option_;
	po::variables_map vm_map_;
	std::string config_path_;
};

}

#include "basic_config_parser_impl.hpp"

#endif

