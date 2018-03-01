/*#############################################################################
 * �ļ���   : basic_config_parser.cpp
 * ������   : tom_liu	
 * ����ʱ�� : 2013��11��14��
 * �ļ����� : ��ȡ�����ļ���Ϣ������Ӧ�������ṩGetValue�Ƚӿڹ������������� 
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <boost/filesystem/fstream.hpp>
#include "basic_config_parser.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] options  ������
 * 		 [in] path �����ļ�·��
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
BasicConfigParser::BasicConfigParser(po::options_description options, 
	const std::string path)
	: option_(options), config_path_(path)
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������ļ�
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
void BasicConfigParser::ParseConfig()
{
	DLOG(INFO)<< "Load configuration from " << config_path_;

	fs::path full_path(config_path_);
	if (!fs::exists(config_path_))
	{
		 DLOG(INFO) << "Configure path(" << config_path_ << ")is't exsit!!!";
		 return ;
	}
	
	fs::ifstream ifs(config_path_);
	po::store(po::parse_config_file(ifs, option_), vm_map_);
	po::notify(vm_map_);
}

}


 

