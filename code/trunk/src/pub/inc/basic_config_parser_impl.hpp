/*#############################################################################
 * �ļ���   : basic_config_parser_impl.hpp
 * ������   : tom_liu	
 * ����ʱ�� : 2013-11-14
 * �ļ����� : basic_config_parser ͷ�ļ���ģ�庯���Ķ���
 * ##########################################################################*/

#ifndef BROADINTER_CONFIG_PARSER_IMPL
#define BROADINTER_CONFIG_PARSER_IMPL

#include <boost/program_options.hpp>

#include "basic_config_parser.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ��GetValue���ã���װ����ϸ��
 * ��  ��: [in] ConfigType type Э������
 *         [in] const string& key  ������
 *         [out] T &t ���ͷ��صĲ���ֵ
 * ����ֵ:
 * ��  ��:
 * ʱ�� 2013��08��19��
 * ���� tom_liu
 * ���� ����
 ----------------------------------------------------------------------------*/
template<typename T>
bool BasicConfigParser::GetValue(const std::string& skey, T &t)
{
	std::map<std::string, po::variable_value>::iterator iter;
	
	iter = vm_map_.find(skey);
	if (iter != vm_map_.end())
	{
		try
		{
			t = boost::any_cast<T>((iter->second).value());
		}
		catch (std::exception& e)
		{
			LOG(ERROR) << "Fail to get value | " << e.what();
			return false;
		}
	}

	return true;
}


}

#endif

