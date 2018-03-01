/*#############################################################################
 * 文件名   : database_logger.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月12日
 * 文件描述 : DatabaseLogger类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_DATABASE_LOGGER
#define BROADINTER_DATABASE_LOGGER

#include <boost/noncopyable.hpp>
#include <mysql/mysql.h>

#include "logger.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 数据库记录器
 * 作  者: teck_zhou
 * 时  间: 2014年05月06日
 ******************************************************************************/
class DatabaseLogger : public Logger
{
public:
	~DatabaseLogger();

	virtual bool Init() override;

	virtual void LogTcpHsTimeoutRecord(const TcpHsTimeoutRecord& record) override;

	virtual void LogTcpHsDelayRecord(const TcpHsDelayRecord& record) override;

	virtual void LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record) override;

	virtual void LogTcpConnAbortRecord(const TcpConnAbortRecord& record) override;

	virtual void LogHttpRespDelayRecord(const HttpRespDelayRecord& record) override;

	virtual void LogHttpDlSpeedRecord(const HttpDlSpeedRecord& record) override;

	virtual void LogHttpAccessRecord(const HttpAccessRecord& record) override;

private:
	bool ConnectToDatabase();
	bool IsAllTablesCreated();
	bool IsTableCreated(const std::string& table_name);

private:
	MYSQL mysql_handle_; // mysql数据库
};

}

#endif