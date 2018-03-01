/*#############################################################################
 * �ļ���   : http_recorder.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��31��
 * �ļ����� : HttpRecorder��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ��¼HTTP�Ự��Ϣ
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 *         [in] delay ʱ����ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpRespDelayRecorder::HttpRespDelayRecorder(const LoggerSP& logger, uint32 delay)
	: logger_(logger), resp_delay_(delay)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼HTTP�Ự��Ϣ
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 *         [in] speed �����ٶ���ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpDlSpeedRecorder::HttpDlSpeedRecorder(const LoggerSP& logger, uint32 speed)
	: logger_(logger), dl_speed_(speed)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼HTTP�Ự��Ϣ
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��12��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpAccessRecorder::HttpAccessRecorder(const LoggerSP& logger) : logger_(logger)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼HTTP������Ϣ
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��12��
 *   ���� teck_zhou
 *   ���� ����
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
