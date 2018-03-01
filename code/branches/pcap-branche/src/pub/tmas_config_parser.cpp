/*#############################################################################
 * �ļ���   : tmas_config_parser.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��1��2��
 * �ļ����� : ��ȡ�����ļ���Ϣ������Ӧ�������ṩGetValue�Ƚӿڹ������������� 
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tmas_config_parser.hpp"

#include "tmas_typedef.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

static int32 kProtocolnum = 1;  //֧�ֵ�Э����� , �������1������global 

static const char* kConfigPaths[] = { GLOBAL_CONFIG_PATH };
static const char* kProtocol[] = { "global" };

#define FILL_GLOBAL_CONFIG_OPTION(config_options) \
	config_options.add_options() \
	("common.capture-interface", po::value<std::string>(), "") \
	("common.capture-filter", po::value<std::string>(), "") \
	("common.packet-process-thread-count", po::value<uint32>(), "") \
	("common.capture-queue-size", po::value<uint32>(), "") \
	("common.capture-queue-count", po::value<uint32>(), "") \
	("protocol.tcp", po::value<std::string>(), "") \
	("protocol.udp", po::value<std::string>(), "") \
	("protocol.http", po::value<std::string>(), "") \
	("ip.enable-ip-checksum", po::value<std::string>(), "") \
	("tcp.complete-handshake-timeout", po::value<uint32>(), "") \
	("tcp.remove-inactive-connection-timeout", po::value<uint32>(), "") \
	("tcp.max-cached-unordered-pkt", po::value<uint32>(), "") \
	("tcp.max-cached-pkt-before-handshake", po::value<uint32>(), "");
	
/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ����������ȡ�����ļ����
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
void TmasConfigParser::Init()
{
	InitGlobalConfig();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��global�����ļ�
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
void TmasConfigParser::InitGlobalConfig()
{
	po::options_description global_option("Configuration");
	BasicConfigParserSP global_parser = CreateBasicParser(GLOBAL_CONFIG, global_option);
	global_parser->ParseConfig();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����BasicConfigParser����ʵ�ʽ���
 * ��  ��: [in]  type  Э������
 *         [out] option Э����صĲ��������� 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��14��
 *   ���� tom_liu
 *   ���� ����
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
 * ��  ��: �ַ���ת�������� "global.global.UGS"ת��  GLOBAL_CONFIG ��  "global.UGS"
 * ��  ��: [in]  key ת��֮ǰ���ַ���
 *         [out] module ����key����������
 *         [out] skey ת��֮��Ĳ��Ҳ���
 * ����ֵ: ת�������ַ����Ƿ�ɹ�
 * ��  ��: 
 * ʱ�� 2013��09��16��
 * ���� tom_liu
 * ���� ����
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

	// i��0��ʼ,globalҲ�����е�һ���ж� 
	int32 i;
	for (i=0; i<kProtocolnum; ++i)
	{
		if (type == kProtocol[i])
		{
			module = static_cast<TmasConfigType>(i);
			break;
		}
	}
	
    // ������ҵ� kProtocolnum, �ͱ�ʾ��û�ҵ� 
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
 * ��  ��: TmasConfigParser �����ӿ�
 * ��  ��: 
 * ����ֵ: TmasConfigParser����
 * ��  ��:
 *   ʱ�� 2013��08��19��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
TmasConfigParser& TmasConfigParser::GetInstance()
{
	static TmasConfigParser instance;
    return instance;
}

}

