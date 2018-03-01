/*#############################################################################
 * 文件名   : http_recorder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月30日
 * 文件描述 : HttpRecorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RECORDER
#define BROADINTER_HTTP_RECORDER

#include <vector>
#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * 描  述: 记录器基类
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class HttpRecorder
{
public:
	virtual ~HttpRecorder() {}

	virtual void OnTick() = 0;
	virtual void RecordHttpRunSession(const HttpRunSessionSP& run_session) = 0;
};

typedef std::vector<HttpRecorder*> RecorderVec;

/*******************************************************************************
 * 描  述: URI记录器
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class UriRecorder : public HttpRecorder
{
public:
	UriRecorder(const string& uri, bool delay, bool speed, uint32 interval);

	virtual void OnTick();
	virtual void RecordHttpRunSession(const HttpRunSessionSP& run_session) override;

private:
	struct HttpUriStat
	{
		HttpUriStat() : total_session_num(0), total_download_size(0)
			, response_delay(0), response_time(0) {}

		uint64 total_session_num;		// 连接的个数
		uint64 total_download_size;		// 总下载大小
		uint64 response_delay;			// 响应时延
		uint64 response_time;			// 接收响应所用时间
	};

private:
	void LogUriStat();
	void ReInit();

private:
	string uri_;

	HttpUriStat uri_stat_;

	bool delay_monitor_;	// 访问时延监测开关
	bool speed_monitor_;	// 下载速度监测开关
	uint32 stat_interval_;	// 统计时间间隔

	uint32 elapsed_tick_;	// 已经流逝的秒数
};

/*******************************************************************************
 * 描  述: Host基类
 * 作  者: teck_zhou
 * 时  间: 2014年03月31日
 ******************************************************************************/
class HostRecorder : public HttpRecorder
{
public:
	HostRecorder(const string& host, bool delay, bool speed, uint32 interval);

	virtual void OnTick();
	virtual void RecordHttpRunSession(const HttpRunSessionSP& run_session) override;

private:
	struct HttpHostStat // 指定domain的统计数据
	{
		HttpHostStat() : total_session_num(0), total_download_size(0)
			, response_delay(0), response_time(0) {}

		uint64 total_session_num;	// 连接的个数
		uint64 total_download_size;	// 总下载大小
		uint64 response_delay;		// 响应时延
		uint64 response_time;		// 接收响应所用时间
	};

private:
	void LogHostStat();
	void ReInit();

private:
	string host_;

	HttpHostStat host_stat_;

	bool delay_monitor_;	// 访问时延监测开关
	bool speed_monitor_;	// 下载速度监测开关

	uint32 stat_interval_;	// 统计时间间隔

	uint32 elpased_tick_;	// 已经流逝的秒数
};

}

#endif
