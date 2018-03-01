/*#############################################################################
 * �ļ���   : tmas_util.cpp
 * ������   : teck_zhou
 * ����ʱ�� : 2014��1��2��
 * �ļ����� : ���߸�����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <pthread.h>
#include <sched.h>

#include "tmas_util.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: �ָ��ַ���
 * ��  ��: [in] str ԭʼ�ַ���
 *         [in] delim �ָ���
 * ����ֵ: �ָ����ַ���
 * ��  ��:
 *   ʱ�� 2013��10��31��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
std::vector<std::string> SplitStr(const std::string& str, char delim)
{
	std::vector<std::string> str_vec;
	std::string::size_type start_pos = 0, end_pos = 0;
	std::string tmp;

	while (start_pos < str.size())
	{
		end_pos = str.find(delim, start_pos);
		if (end_pos == std::string::npos)
		{
			tmp = str.substr(start_pos);
			str_vec.push_back(tmp);
			break;
		}
		
		if (start_pos != end_pos) 
		{
			tmp = str.substr(start_pos, end_pos - start_pos);
			str_vec.push_back(tmp);
		}

		start_pos = end_pos + 1;
	}

	return str_vec;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ӡMAC��ַ
 * ��  ��: [in] mac MAC
 * ����ֵ: MAC�ַ�����ʾ
 * ��  ��:
 *   ʱ�� 2014��01��23��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
std::string GetMacStr(const char* mac)
{
	std::string mac_str;

	for (int i = 0; i < 6; i++)
	{
		mac_str.append(ToHexStr(*(mac + i)));

		if (i < 5 && i % 2 != 0)
		{
			mac_str.append(":");
		}
	}

	return mac_str;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʱ���ת��Ϊ����
 * ��  ��: [in] start ��ʼʱ��
 *         [in] end ����ʱ��
 * ����ֵ: ����������Ϊ����
 * ��  ��:
 *   ʱ�� 2013��10��28��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
int64 GetDurationSeconds(const ptime& start, const ptime& end)
{
	time_duration duration = end - start;
	return FromDurationToSeconds(duration);
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʱ���ת��Ϊ������
 * ��  ��: [in] start ��ʼʱ��
 *         [in] end ����ʱ��
 * ����ֵ: ������������Ϊ����
 * ��  ��:
 *   ʱ�� 2014��02��18��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
int64 GetDurationMilliSeconds(const ptime& start, const ptime& end)
{
	time_duration duration = end - start;
	return duration.total_milliseconds();
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʱ���ת��Ϊ����
 * ��  ��: [in] duration ʱ���
 * ����ֵ: ����������Ϊ����
 * ��  ��:
 *   ʱ�� 2013��10��28��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
int64 FromDurationToSeconds(const time_duration& duration)
{
	int64 hours = duration.hours();
	int64 minutes = duration.minutes();
	int64 seconds = duration.seconds();

	return (hours * 3600 + minutes * 60 + seconds);
}

/*------------------------------------------------------------------------------
 * ��  ��: �ҵ���һ��delim��λ��
 * ��  ��: [in] begin ������ʼλ��
 *         [in] end ���ݽ���λ��
 *         [in] delim �ָ���
 * ����ֵ: �ҵ���λ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
char* FindUntilPos(const char* begin, const char* end, char delim)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && *index != delim; ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * ��  ��: �ҵ���һ��delim1����delim2��λ��
 * ��  ��: [in] begin ������ʼλ��
 *         [in] end ���ݽ���λ��
 *         [in] delim1 ��һ���ָ���
 *         [in] delim2 �ڶ����ָ���
 * ����ֵ: �ҵ���λ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
char* FindUntilPos2(const char* begin, const char* end, char delim1, char delim2)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && *index != delim1 && *index != delim2; ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * ��  ��: �ҵ���һ����delim��λ��
 * ��  ��: [in] begin ������ʼλ��
 *         [in] end ���ݽ���λ��
 *         [in] delim �ָ���
 * ����ֵ: �ҵ���λ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
char* FindUntilNotPos(const char* begin, const char* end, char delim)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && *index == delim; ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * ��  ��: �ҵ���һ����delim1�ҷ�delim2��λ��
 * ��  ��: [in] begin ������ʼλ��
 *         [in] end ���ݽ���λ��
 *         [in] delim1 ��һ���ָ���
 *         [in] delim2 �ڶ����ָ���
 * ����ֵ: �ҵ���λ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
char* FindUntilNotPos2(const char* begin, const char* end, char delim1, char delim2)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && (*index == delim1 || *index == delim2); ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * ��  ��: ������������ת����16�����ַ���
 * ��  ��: [in]data ��ת���Ķ���������
 *         [in]lower_case ת���ɵ�16�����ַ����е��ַ��Ƿ���Сд
 * ����ֵ: ת�����16�����ַ���
 * ��  ��:
 *   ʱ�� 2013.10.14
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
std::string ToHex(const std::string& data, bool lower_case)
{
    std::string hex_data; //16�����ַ���
    for (char ch : data)
    {
        hex_data.push_back(ToHex((ch >> 4) & 0xf, lower_case)); //����4λת����16�����ַ�
        hex_data.push_back(ToHex(ch & 0xf, lower_case)); //����4λת����16�����ַ�
    }

    return hex_data;
}

/*------------------------------------------------------------------------------
 * ��  ��: ������������ת����16�����ַ���
 * ��  ��: [in] t BOOST�߳̾��
 *         [in] cpu_core_index CPU��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��21��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
bool BindThreadToCpuCore(const ThreadSP& t, uint8 cpu_core_index)
{
	cpu_set_t mask;
	pthread_t handle = t->native_handle();

	CPU_ZERO(&mask);
	CPU_SET(cpu_core_index, &mask);

	if (pthread_setaffinity_np(handle, sizeof(mask), &mask) < 0) 
	{
		return false;
	}
	else
	{
		return true;
	}
}

}