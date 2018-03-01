/*#############################################################################
 * 文件名   : tmas_config_parser.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年1月2日
 * 文件描述 : 读取配置文件信息并做相应解析，提供GetValue等接口供其他函数调用 
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tmas_config_parser.hpp"

#include "tmas_typedef.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

static int32 kProtocolnum = 1;  //支持的协议个数 , 个数会加1算上了global 

static const char* kConfigPaths[] = { GLOBAL_CONFIG_PATH };
static const char* kProtocol[] = { "global" };

#define FILL_GLOBAL_CONFIG_OPTION(config_options) \
	config_options.add_options() \
	("common.log-record-type", po::value<std::string>(), "") \
	("capture.packet-capture-type", po::value<std::string>(), "") \
	("capture.capture-interface", po::value<std::string>(), "") \
	("capture.capture-filter", po::value<std::string>(), "") \
	("capture.bind-capture-thread-to-cpu", po::value<std::string>(), "") \
	("netmap.packet-process-thread-count", po::value<uint32>(), "") \
	("netmap.packet-process-fifo-size", po::value<uint32>(), "") \
	("netmap.packet-dispatch-policy", po::value<std::string>(), "") \
	("netmap.bind-process-thread-to-cpu", po::value<std::string>(), "") \
	("netmap.batch-dispatch-packet", po::value<std::string>(), "") \
	("protocol.tcp", po::value<std::string>(), "") \
	("protocol.udp", po::value<std::string>(), "") \
	("protocol.http", po::value<std::string>(), "") \
	("ip.enable-ip-checksum", po::value<std::string>(), "") \
	("tcp.handshake-timeout", po::value<uint32>(), "") \
	("tcp.connection-timeout", po::value<uint32>(), "") \
	("tcp.max-cached-unordered-pkt", po::value<uint32>(), "") \
	("tcp.max-cached-pkt-before-handshake", po::value<uint32>(), "") \
	("database.host", po::value<std::string>(), "") \
	("database.port", po::value<uint32>(), "") \
	("database.user", po::value<std::string>(), "") \
	("database.password", po::value<std::string>(), "") \
	("database.name", po::value<std::string>(), "") \
	("debug.print-captured-packet-number", po::value<std::string>(), "") \
	("debug.print-received-packet-number", po::value<std::string>(), "") \
	("debug.print-interval", po::value<uint32>(), "");
	
/*-----------------------------------------------------------------------------
 * 描  述: 初始化函数，读取配置文件入口
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2013年11月13日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TmasConfigParser::Init()
{
	InitGlobalConfig();
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化global配置文件
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2013年11月13日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TmasConfigParser::InitGlobalConfig()
{
	po::options_description global_option("Configuration");
	BasicConfigParserSP global_parser = CreateBasicParser(GLOBAL_CONFIG, global_option);
	global_parser->ParseConfig();
}

/*-----------------------------------------------------------------------------
 * 描  述: 生产BasicConfigParser用于实际解析
 * 参  数: [in]  type  协议类型
 *         [out] option 协议相关的参数配置项 
 * 返回值:
 * 修  改:
 *   时间 2013年11月14日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
BasicConfigParserSP TmasConfigParser::CreateBasicParser(TmasConfigType type, 
														po::options_description& option)
{
	BasicConfigParserSP basic_parser_;
	switch(type)
	{
		case GLOBAL_CONFIG:
			FILL_GLOBAL_CONFIG_OPTION(option);
			basic_parser_.reset(new BasicConfigParser(option, kConfigPaths[type]));
			basic_config_map_.insert(std::make_pair(type, basic_parser_));
			break;
			
		default:
			LOG(ERROR) << "Invalid configuration";
			break;
	}

	basic_config_map_.insert(std::make_pair(type, basic_parser_));
	return basic_parser_;
}

/*-----------------------------------------------------------------------------
 * 描  述: 字符串转换函数， "global.global.UGS"转成  GLOBAL_CONFIG 和  "global.UGS"
 * 参  数: [in]  key 转换之前的字符串
 *         [out] module 查找key所属的类型
 *         [out] skey 转换之后的查找参数
 * 返回值: 转换查找字符串是否成功
 * 修  改: 
 * 时间 2013年09月16日
 * 作者 tom_liu
 * 描述 创建
 ----------------------------------------------------------------------------*/
bool TmasConfigParser::ParseConfigTypeAndConfigItem(const std::string& key, 
	                                                TmasConfigType& module, 
										            std::string& skey)
{
	bool ret = false;
	std::string::size_type pos = key.find('.');
	if (pos == key.npos)
	{
		LOG(ERROR) << "Can't find '.' in the key string";
		return ret;
	}
	std::string type = key.substr(0, pos);

	// i从0开始,global也是其中的一个判断 
	int32 i;
	for (i=0; i<kProtocolnum; ++i)
	{
		if (type == kProtocol[i])
		{
			module = static_cast<TmasConfigType>(i);
			break;
		}
	}
	
    // 如果查找到 kProtocolnum, 就表示还没找到 
	if (i == kProtocolnum)
	{
		LOG(ERROR) << "Don't have this module | key:" << key;
		return ret;
	}
	
    ret = true;
	skey = key.substr(pos+1);

	return ret;
}

/*-----------------------------------------------------------------------------
 * 描  述: TmasConfigParser 单例接口
 * 参  数: 
 * 返回值: TmasConfigParser引用
 * 修  改:
 *   时间 2013年08月19日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
TmasConfigParser& TmasConfigParser::GetInstance()
{
	static TmasConfigParser instance;
    return instance;
}

}

