/*#############################################################################
 * �ļ���   : file_logger.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��06��
 * �ļ����� : �ļ���¼������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "file_logger.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
FileLogger::~FileLogger()
{
	tcp_hs_timeout_ofs_.close();
	tcp_hs_delay_ofs_.close();
	tcp_conn_timeout_ofs_.close();
	tcp_conn_abort_ofs_.close();
	http_resp_delay_ofs_.close();
	http_dl_speed_ofs_.close();
	http_access_ofs_.close();
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool FileLogger::Init()
{
	tcp_hs_timeout_ofs_.open("tcp_hs_timeout.log", std::ios_base::app);

	tcp_hs_delay_ofs_.open("tcp_hs_delay.log", std::ios_base::app);

	tcp_conn_timeout_ofs_.open("tcp_conn_timeout.log", std::ios_base::app);

	tcp_conn_abort_ofs_.open("tcp_conn_abort.log", std::ios_base::app);

	http_resp_delay_ofs_.open("http_resp_delay.log", std::ios_base::app);

	http_dl_speed_ofs_.open("http_dl_speed.log", std::ios_base::app);

	// http_access_ofs_.open("http_access.log", std::ios_base::app);

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogTcpHsTimeoutRecord(const TcpHsTimeoutRecord& record)
{
	tcp_hs_timeout_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< record.timeout_value
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼TCP����ʱ����Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogTcpHsDelayRecord(const TcpHsDelayRecord& record)
{
	tcp_hs_delay_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< record.hs_delay
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ӳ�ʱ��Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record)
{
	tcp_conn_timeout_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼TCP�����쳣�ж���Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogTcpConnAbortRecord(const TcpConnAbortRecord& record)
{
	tcp_conn_abort_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼HTTP��Ӧʱ����Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogHttpRespDelayRecord(const HttpRespDelayRecord& record)
{
	http_resp_delay_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< record.host << " " 
		<< record.uri << " "
		<< record.resp_delay << " "
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼HTTP�����ٶ���Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogHttpDlSpeedRecord(const HttpDlSpeedRecord& record)
{
	http_dl_speed_ofs_ << " "
		<< record.record_time << " "
		<< record.conn_id << " "
		<< record.host << " "
		<< record.uri << " "
		<< record.dl_speed
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼HTTP���ʼ�¼��Ϣ
 * ��  ��: [in] record ��¼��Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��12��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void FileLogger::LogHttpAccessRecord(const HttpAccessRecord& record)
{
    static const char kSeperator = '\001';

    if (IsNewAccessRecordFileNeeded())
        CreateNewAccessRecordFile();

	http_access_ofs_ << record.access_time << kSeperator 
        << record.request_ip << kSeperator
        << record.request_port << kSeperator
        << record.response_ip << kSeperator
        << record.response_port << kSeperator
        << record.uri << kSeperator
        << record.status_code << kSeperator
        << record.response_delay << kSeperator
        << record.response_elapsed << kSeperator
        << record.response_size << '\n' << std::ends;
}

bool FileLogger::IsNewAccessRecordFileNeeded()
{
    static const uint32 kMaxFileSize = 512 * 1024 * 1024;  // ��־�ļ����Ϊ512M

    return !http_access_ofs_.is_open() || http_access_ofs_.tellp() > kMaxFileSize;
}

void FileLogger::CreateNewAccessRecordFile()
{
    http_access_ofs_.close();
    http_access_ofs_.open(boost::lexical_cast<std::string>(time(nullptr)).c_str());
}

}
