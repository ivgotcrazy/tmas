/*#############################################################################
 * �ļ���   : database_logger.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : DatabaseLogger������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_DATABASE_LOGGER
#define BROADINTER_DATABASE_LOGGER

#include <boost/noncopyable.hpp>
#include <mysql/mysql.h>

#include "logger.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���ݿ��¼��
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��06��
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
	MYSQL mysql_handle_; // mysql���ݿ�
};

}

#endif