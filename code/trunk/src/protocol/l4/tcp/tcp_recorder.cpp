/*#############################################################################
 * 文件名   : tcp_recorder.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : TcpReorder类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "tcp_recorder.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 记录连接信息
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void TcpRecorder::Record(const TcpRecordInfo& record_info)
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
HsDelayRecorder::HsDelayRecorder(const LoggerSP& logger, uint32 delay)
	: logger_(logger), delay_(delay)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录连接信息
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HsDelayRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// 连接正常关闭才需要记录握手时延
	if (record_info.conn_event != TCE_CONN_CLOSE) return;

	// 只记录比预设值大的握手时延
	if (record_info.hs_delay < delay_) return;

	TcpHsDelayRecord delay_record;

	delay_record.conn_id = record_info.conn_id;
	delay_record.hs_delay = record_info.hs_delay;
	delay_record.record_time = 0;

	logger_->LogTcpHsDelayRecord(delay_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 *         [in] timeout 超时值
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HsTimeoutRecorder::HsTimeoutRecorder(const LoggerSP& logger, uint32 timeout)
	: logger_(logger), timeout_(timeout)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录连接信息
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HsTimeoutRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// 只关注握手超时事件
	if (record_info.conn_event != TCE_HS_TIMEOUT) return;

	TcpHsTimeoutRecord timeout_record;

	timeout_record.conn_id = record_info.conn_id;
	timeout_record.timeout_value = timeout_;
	timeout_record.record_time = 0;

	logger_->LogTcpHsTimeoutRecord(timeout_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
ConnAbortRecorder::ConnAbortRecorder(const LoggerSP& logger) : logger_(logger)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录连接信息
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void ConnAbortRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// 只关注TCP连接异常中断事件
	if (record_info.conn_event != TCE_CONN_ABORT) return;

	TcpConnAbortRecord abort_record;

	abort_record.conn_id = record_info.conn_id;
	abort_record.record_time = 0;

	logger_->LogTcpConnAbortRecord(abort_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 *         [in] timeout 超时值
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
ConnTimeoutRecorder::ConnTimeoutRecorder(const LoggerSP& logger, uint32 timeout)
	: logger_(logger), timeout_(timeout)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 记录连接信息
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void ConnTimeoutRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// 只关注TCP连接超时事件
	if (record_info.conn_event != TCE_CONN_TIMEOUT) return;

	TcpConnTimeoutRecord timeout_record;

	timeout_record.conn_id = record_info.conn_id;
	timeout_record.timeout_value = timeout_;
	timeout_record.record_time = 0;

	logger_->LogTcpConnTimeoutRecord(timeout_record);
}

}