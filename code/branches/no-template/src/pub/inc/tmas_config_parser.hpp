/*#############################################################################
 * 文件名   : tmas_config_parser.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年1月2日
 * 文件描述 : TmasConfigParser声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_CONFIG_PARSER
#define BROADINTER_TMAS_CONFIG_PARSER

#include <boost/program_options.hpp>
#include "tmas_typedef.hpp"
#include "basic_config_parser.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述：tmas配置解析 
 * 作者：teck_zhou
 * 时间：2014/01/02
 *****************************************************************************/
class TmasConfigParser
{
public:
	void Init();

	template<typename T> bool GetValue(const std::string& key, T &t);

	static TmasConfigParser& GetInstance();

private:
	enum TmasConfigType
	{
		GLOBAL_CONFIG,
	};
	
private:
	TmasConfigParser(){}

	void InitGlobalConfig();

	bool ParseConfigTypeAndConfigItem(const std::string& key, TmasConfigType& module, std::string& skey);

	BasicConfigParserSP CreateBasicParser(TmasConfigType type, po::options_description& option);

private:
	typedef std::map<TmasConfigType, BasicConfigParserSP> BasicParserMap;
	BasicParserMap basic_config_map_;
};

}

#include "tmas_config_parser_impl.hpp"

#endif 
