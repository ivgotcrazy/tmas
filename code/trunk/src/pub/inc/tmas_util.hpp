/*#############################################################################
 * 文件名   : tmas_util.hpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年1月2日
 * 文件描述 : 工具辅助类
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_UTIL
#define BROADINTER_TMAS_UTIL

#include <vector>
#include <string>

#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

//==============================================================================

// 从配置文件中获取字符串
#define GET_CONFIG_STR(ConfigParser, item, para) \
{ \
	ConfigParser& configer = ConfigParser::GetInstance(); \
	bool ret = configer.GetValue<std::string>(item, (para)); \
	if (!ret) \
	{ \
		LOG(FATAL) << "Parse config error | " << item; \
	} \
}

// 从配置文件中获取bool值
#define GET_CONFIG_BOOL(ConfigParser, item, para) \
{ \
	ConfigParser& configer = ConfigParser::GetInstance(); \
	std::string config_str; \
	bool ret = configer.GetValue<std::string>(item, config_str); \
	if (!ret || (config_str != "yes" && config_str != "no")) \
	{ \
		LOG(FATAL) << "Parse config error | " << item; \
	} \
	else if (config_str == "yes") \
	{ \
		para = true; \
	} \
	else \
	{ \
		para = false; \
	} \
}

// 从配置文件中获取整形
#define GET_CONFIG_INT(ConfigParser, item, para) \
{ \
	ConfigParser& configer = ConfigParser::GetInstance(); \
	bool ret = configer.GetValue<uint32>(item, (para)); \
	if (!ret) \
	{ \
		LOG(FATAL) << "Parse config error | " << item; \
	} \
}

// 获取配置项
#define GET_TMAS_CONFIG_INT(item, para) GET_CONFIG_INT(TmasConfigParser, item, para)
#define GET_TMAS_CONFIG_STR(item, para) GET_CONFIG_STR(TmasConfigParser, item, para)
#define GET_TMAS_CONFIG_BOOL(item, para) GET_CONFIG_BOOL(TmasConfigParser, item, para)


//==============================================================================

std::vector<std::string> SplitStr(const std::string& str, char delim);

std::string GetMacStr(const char* mac);

int64 FromDurationToSeconds(const time_duration& duration);

int64 GetDurationSeconds(const ptime& start, const ptime& end);

int64 GetDurationMilliSeconds(const ptime& start, const ptime& end);

char* FindUntilPos(const char* begin, const char* end, char delim);

char* FindUntilPos2(const char* begin, const char* end, char delim1, char delim2);

char* FindUntilNotPos(const char* begin, const char* end, char delim);

char* FindUntilNotPos2(const char* begin, const char* end, char delim1, char delim2);

std::string ToHex(const std::string& data, bool lower_case = false);

bool BindThreadToCpuCore(const ThreadSP& t, uint8 cpu_core_index);

//==============================================================================

inline ptime TimeNow() 
{ 
	return boost::date_time::second_clock<ptime>::universal_time(); 
}

inline ptime MicroTimeNow()
{
	return boost::date_time::microsec_clock<ptime>::universal_time();
}

inline uint64 GetMicroSecond()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	return tv.tv_sec * 1000000 + tv.tv_usec;
}

inline std::string GetLocalTime()
{
	typedef boost::date_time::local_adjustor<ptime, +8, boost::posix_time::no_dst> cn_timezone;

	const ptime& now = MicroTimeNow(); 
	ptime local_now = cn_timezone::utc_to_local(now);

	return to_simple_string(local_now.time_of_day());
}

inline std::string to_string(unsigned long ip)
{
	return boost::asio::ip::address_v4(ip).to_string();
}

inline std::string ToHexStr(char ch, bool lower_case = false)
{
	std::string result;

    static const char kTable[2][16] = {
        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'},
        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'} };

    result.push_back(kTable[!lower_case][(ch >> 4) & 0xf]);
	result.push_back(kTable[!lower_case][ch & 0xf]);

	return result;     
}

inline char ToHex(char ch, bool lower_case = false)
{
    static const char kTable[2][16] = {
        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'},
        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'} };

    return kTable[!lower_case][ch & 0xf];          
}

inline bool IsDigit(char c) 
{ 
	return c >= '0' && c <= '9'; 
}

inline bool IsChar(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline char ToLower(char c)
{
	return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

inline void ToLower(char* c)
{
	if (*c >= 'A' && *c <= 'Z')
	{
		*c = *c - 'A' + 'a';
	}
}

inline char ToUpper(char c)
{
	return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

inline void ToUpper(char *c)
{
	if (*c >= 'a' && *c <= 'z')
	{
		*c = *c - 'a' + 'A';
	}
}

}

#endif