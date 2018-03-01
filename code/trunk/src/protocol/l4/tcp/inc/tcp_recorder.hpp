/*#############################################################################
 * 文件名   : tcp_recorder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : TcpReorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_RECORDER
#define BROADINTER_TCP_RECORDER

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "tcp_typedef.hpp"
#include "logger.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: TCP记录器基类
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpRecorder : public boost::noncopyable
{
public:
	void Record(const TcpRecordInfo& record_info);

	void SetNextRecorder(const TcpRecorderSP& next) { next_recorder_ = next; }

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) = 0;

private:
	TcpRecorderSP next_recorder_;
};

/******************************************************************************
 * 描述: TCP握手时延记录器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HsDelayRecorder : public TcpRecorder
{
public:
	HsDelayRecorder(const LoggerSP& logger, uint32 delay);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 delay_;
};

/******************************************************************************
 * 描述: TCP握手超时记录器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HsTimeoutRecorder : public TcpRecorder
{
public:
	HsTimeoutRecorder(const LoggerSP& logger, uint32 timeout);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 timeout_;
};

/******************************************************************************
 * 描述: TCP连接异常断开记录器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class ConnAbortRecorder : public TcpRecorder
{
public:
	ConnAbortRecorder(const LoggerSP& logger);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
};

/******************************************************************************
 * 描述: TCP连接超时记录器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class ConnTimeoutRecorder : public TcpRecorder
{
public:
	ConnTimeoutRecorder(const LoggerSP& logger, uint32 timeout);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 timeout_;
};

}

#endif