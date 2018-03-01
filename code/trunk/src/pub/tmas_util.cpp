/*#############################################################################
 * 文件名   : tmas_util.cpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年1月2日
 * 文件描述 : 工具辅助类
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <pthread.h>
#include <sched.h>

#include "tmas_util.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 分割字符串
 * 参  数: [in] str 原始字符串
 *         [in] delim 分隔符
 * 返回值: 分割后的字符串
 * 修  改:
 *   时间 2013年10月31日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 打印MAC地址
 * 参  数: [in] mac MAC
 * 返回值: MAC字符串表示
 * 修  改:
 *   时间 2014年01月23日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 将时间段转换为秒数
 * 参  数: [in] start 开始时间
 *         [in] end 结束时间
 * 返回值: 秒数，可能为负数
 * 修  改:
 *   时间 2013年10月28日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
int64 GetDurationSeconds(const ptime& start, const ptime& end)
{
	time_duration duration = end - start;
	return FromDurationToSeconds(duration);
}

/*------------------------------------------------------------------------------
 * 描  述: 将时间段转换为毫秒数
 * 参  数: [in] start 开始时间
 *         [in] end 结束时间
 * 返回值: 豪秒数，可能为负数
 * 修  改:
 *   时间 2014年02月18日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
int64 GetDurationMilliSeconds(const ptime& start, const ptime& end)
{
	time_duration duration = end - start;
	return duration.total_milliseconds();
}

/*------------------------------------------------------------------------------
 * 描  述: 将时间段转换为秒数
 * 参  数: [in] duration 时间段
 * 返回值: 秒数，可能为负数
 * 修  改:
 *   时间 2013年10月28日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
int64 FromDurationToSeconds(const time_duration& duration)
{
	int64 hours = duration.hours();
	int64 minutes = duration.minutes();
	int64 seconds = duration.seconds();

	return (hours * 3600 + minutes * 60 + seconds);
}

/*------------------------------------------------------------------------------
 * 描  述: 找到第一个delim的位置
 * 参  数: [in] begin 数据起始位置
 *         [in] end 数据结束位置
 *         [in] delim 分隔符
 * 返回值: 找到的位置
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
char* FindUntilPos(const char* begin, const char* end, char delim)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && *index != delim; ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * 描  述: 找到第一个delim1或者delim2的位置
 * 参  数: [in] begin 数据起始位置
 *         [in] end 数据结束位置
 *         [in] delim1 第一个分隔符
 *         [in] delim2 第二个分隔符
 * 返回值: 找到的位置
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
char* FindUntilPos2(const char* begin, const char* end, char delim1, char delim2)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && *index != delim1 && *index != delim2; ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * 描  述: 找到第一个非delim的位置
 * 参  数: [in] begin 数据起始位置
 *         [in] end 数据结束位置
 *         [in] delim 分隔符
 * 返回值: 找到的位置
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
char* FindUntilNotPos(const char* begin, const char* end, char delim)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && *index == delim; ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * 描  述: 找到第一个非delim1且非delim2的位置
 * 参  数: [in] begin 数据起始位置
 *         [in] end 数据结束位置
 *         [in] delim1 第一个分隔符
 *         [in] delim2 第二个分隔符
 * 返回值: 找到的位置
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
char* FindUntilNotPos2(const char* begin, const char* end, char delim1, char delim2)
{
	char* index = const_cast<char*>(begin);
	for (; index != end && (*index == delim1 || *index == delim2); ++index);

	return index;
}

/*------------------------------------------------------------------------------
 * 描  述: 将二进制数据转换成16进制字符串
 * 参  数: [in]data 被转换的二进制数据
 *         [in]lower_case 转换成的16进制字符串中的字符是否是小写
 * 返回值: 转换后的16进制字符串
 * 修  改:
 *   时间 2013.10.14
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
std::string ToHex(const std::string& data, bool lower_case)
{
    std::string hex_data; //16进制字符串
    for (char ch : data)
    {
        hex_data.push_back(ToHex((ch >> 4) & 0xf, lower_case)); //将高4位转换成16进制字符
        hex_data.push_back(ToHex(ch & 0xf, lower_case)); //将低4位转换成16进制字符
    }

    return hex_data;
}

/*------------------------------------------------------------------------------
 * 描  述: 将二进制数据转换成16进制字符串
 * 参  数: [in] t BOOST线程句柄
 *         [in] cpu_core_index CPU核心索引
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月21日
 *   作者 teck_zhou
 *   描述 创建
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