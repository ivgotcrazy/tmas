/*#############################################################################
 * �ļ���   : file_logger.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��06��
 * �ļ����� : �ļ���¼������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_FILE_LOGGER
#define BROADINTER_FILE_LOGGER

#include <fstream>

#include "logger.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: �ļ���¼����������д�뵽�ļ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��06��
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
