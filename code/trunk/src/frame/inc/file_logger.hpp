/*#############################################################################
 * 文件名   : file_logger.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月06日
 * 文件描述 : 文件记录器声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_FILE_LOGGER
#define BROADINTER_FILE_LOGGER

#include <fstream>

#include "logger.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 文件记录器，将数据写入到文件
 * 作  者: teck_zhou
 * 时  间: 2014年05月06日
 ******************************************************************************/
class FileLogger : public Logger
{
public:
	~FileLogger();

	virtual bool Init() override;

	virtual void LogTcpHsTimeoutRecord(const TcpHsTimeoutRecord& record) override;

	virtual void LogTcpHsDelayRecord(const TcpHsDelayRecord& record) override;

	virtual void LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record) override;

	virtual void LogTcpConnAbortRecord(const TcpConnAbortRecord& record) override;

	virtual void LogHttpRespDelayRecord(const HttpRespDelayRecord& record) override;

	virtual void LogHttpDlSpeedRecord(const HttpDlSpeedRecord& record) override;

	virtual void LogHttpAccessRecord(const HttpAccessRecord& record) override;

private:
    bool IsNewAccessRecordFileNeeded();
    void CreateNewAccessRecordFile();

private:
	std::ofstream tcp_hs_timeout_ofs_;

	std::ofstream tcp_hs_delay_ofs_;

	std::ofstream tcp_conn_timeout_ofs_;

	std::ofstream tcp_conn_abort_ofs_;

	std::ofstream http_resp_delay_ofs_;

	std::ofstream http_dl_speed_ofs_;

	std::ofstream http_access_ofs_;
};

}

#endif
