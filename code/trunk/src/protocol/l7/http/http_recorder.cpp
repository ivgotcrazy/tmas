/*#############################################################################
 * 文件名   : http_recorder.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月31日
 * 文件描述 : HttpRecorder类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <fstream>

#include "http_recorder.hpp"
#include "http_typedef.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "http_run_session.hpp"
#include "record.hpp"
#include "logger.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 记录HTTP会话信息
 * 参  数: [in] record_info 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRecorder::Record(const HttpRecordInfo& record_info)
{
	DoRecord(record_info);

	if (next_recorder_)
	{
		next_recorder_->Record(record_info);
	}
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 *         [in] delay 时延阈值
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpRespDelayRecorder::HttpRespDelayRecorder(const LoggerSP& logger, uint32 delay)
	: logger_(logger), resp_delay_(delay)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录HTTP会话信息
 * 参  数: [in] record_info 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRespDelayRecorder::DoRecord(const HttpRecordInfo& record_info)
{
	TMAS_ASSERT(record_info.response_begin >= record_info.request_end);

	uint32 resp_delay = (record_info.response_begin - record_info.request_end) / 1000;

	if (resp_delay < resp_delay_) return;

	HttpRespDelayRecord delay_record;

	delay_record.conn_id = record_info.conn_id;
	delay_record.uri = record_info.request_info.request_line.uri;
	delay_record.resp_delay = resp_delay;
	delay_record.record_time = 0;
	
	auto iter = record_info.request_info.header.find("host");
	if (iter != record_info.request_info.header.end())
	{
		delay_record.host = iter->second;
	}

	logger_->LogHttpRespDelayRecord(delay_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 *         [in] speed 下载速度阈值
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpDlSpeedRecorder::HttpDlSpeedRecorder(const LoggerSP& logger, uint32 speed)
	: logger_(logger), dl_speed_(speed)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录HTTP会话信息
 * 参  数: [in] record_info 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpDlSpeedRecorder::DoRecord(const HttpRecordInfo& record_info)
{
	TMAS_ASSERT(record_info.response_end >= record_info.response_begin);

	uint32 dl_speed = 0;
	uint32 resp_time = (record_info.response_end - record_info.response_begin);
	if (resp_time != 0)
	{
		dl_speed = record_info.response_size * 1000 * 1000 / resp_time;
	}

	if (dl_speed > dl_speed_) return;

	HttpDlSpeedRecord speed_record;

	speed_record.conn_id = record_info.conn_id;
	speed_record.uri = record_info.request_info.request_line.uri;
	speed_record.dl_speed = dl_speed;
	speed_record.record_time = 0;

	auto iter = record_info.request_info.header.find("host");
	if (iter != record_info.request_info.header.end())
	{
		speed_record.host = iter->second;
	}

	logger_->LogHttpDlSpeedRecord(speed_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 * 返回值: 
 * 修  改:
 *   时间 2014年05月12日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpAccessRecorder::HttpAccessRecorder(const LoggerSP& logger) : logger_(logger)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录HTTP访问信息
 * 参  数: [in] record_info 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月12日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpAccessRecorder::DoRecord(const HttpRecordInfo& record_info)
{
	HttpAccessRecord access_record;

    access_record.access_time = record_info.request_end;
	access_record.uri = record_info.request_info.request_line.uri;
	access_record.status_code = record_info.status_code;
	access_record.request_header = record_info.request_header;
	access_record.response_header = record_info.response_header;
	access_record.response_delay = record_info.response_begin - record_info.request_end;
	access_record.response_elapsed = record_info.response_end - record_info.response_begin;
	access_record.response_size = record_info.response_size;

    access_record.send_size = record_info.request.size()
        + record_info.request_header.size() + record_info.request_size;
    access_record.receive_size = record_info.response.size()
        + record_info.response_header.size() + record_info.response_size;

    const ConnId& id = record_info.conn_id;
    if (record_info.request_direction == PKT_DIR_S2B)
    {
        access_record.request_ip = id.smaller_ip;
        access_record.request_port = id.smaller_port;
        access_record.response_ip = id.bigger_ip;
        access_record.response_port = id.bigger_port;
    }
    else  // record_info.request_direction == PKT_DIR_B2S
    {
        access_record.request_ip = id.bigger_ip;
        access_record.request_port = id.bigger_port;
        access_record.response_ip = id.smaller_ip;
        access_record.response_port = id.smaller_port;
    }

	logger_->LogHttpAccessRecord(access_record);
}

}
