/*#############################################################################
 * �ļ���   : tmas_config_parser_impl.hpp
 * ������   : tom_liu	
 * ����ʱ�� : 2013-8-19
 * �ļ����� : tmas_config_parser ͷ�ļ���ģ�庯���Ķ���
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_CONFIG_PARSER_IMPL
#define BROADINTER_TMAS_CONFIG_PARSER_IMPL

#include "tmas_config_parser.hpp"

namespace BroadInter
{
/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ������ڣ��ô˺������в�����ȡ
 * ��  ��: [in] const string& key  ������
 *         [out] T &t ���ͷ��صĲ���ֵ
 * ����ֵ: ��ȡ�����Ƿ�ɹ�
 * ��  ��:
 * ʱ�� 2013��08��19��
 * ���� tom_liu
 * ���� ����
 ----------------------------------------------------------------------------*/
template<typename T>
bool TmasConfigParser::GetValue(const std::string& key, T &t)
{
	bool ret = false;
	TmasConfigType type;
	std::string child_key;
	ParseConfigTypeAndConfigItem(key, type, child_key);

	BasicParserMap::iterator it = basic_config_map_.find(type);
	if (it == basic_config_map_.end())
	{
		LOG(ERROR) << "Can't find the basic config parser ";
	 	return ret;
	}
	
	ret = it->second->GetValue(child_key, t);	
	return ret;
}

}


#endif


