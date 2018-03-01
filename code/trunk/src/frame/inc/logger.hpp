/*#############################################################################
 * 文件名   : logger.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月06日
 * 文件描述 : 数据记录抽象接口声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_LOGGER
#define BROADINTER_LOGGER

#include "record.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 数据记录器抽象接口
 * 作  者: teck_zhou
 * 时  间: 2014年05月06日
 ******************************************************************************/
class Logger : public boost::noncopyable
{
public:
	virtual bool Init() = 0;

	virtual void LogTcpHsTimeoutRecord(const TcpHsTimeoutRecord& record) = 0;

	virtual void LogTcpHsDelayRecord(const TcpHsDelayRecord& record) = 0;

	virtual void LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record) = 0;

	virtual void LogTcpConnAbortRecord(const TcpConnAbortRecord& record) = 0;

	virtual void LogHttpRespDelayRecord(const HttpRespDelayRecord& record) = 0;

	virtual void LogHttpDlSpeedRecord(const HttpDlSpeedRecord& record) = 0;

	virtual void LogHttpAccessRecord(const HttpAccessRecord& record) = 0;
};


}

#endif