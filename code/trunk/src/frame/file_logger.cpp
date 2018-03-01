/*#############################################################################
 * 文件名   : file_logger.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月06日
 * 文件描述 : 文件记录器声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "file_logger.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 记录TCP握手超时信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 记录TCP握手时延信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 记录TCP连接超时信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void FileLogger::LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record)
{
	tcp_conn_timeout_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * 描  述: 记录TCP连接异常中断信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void FileLogger::LogTcpConnAbortRecord(const TcpConnAbortRecord& record)
{
	tcp_conn_abort_ofs_ << " "
		<< record.record_time << " "
		<< std::setw(36) << record.conn_id << " "
		<< std::endl;
}

/*------------------------------------------------------------------------------
 * 描  述: 记录HTTP响应时延信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 记录HTTP下载速度信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 记录HTTP访问记录信息
 * 参  数: [in] record 记录信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月12日
 *   作者 teck_zhou
 *   描述 创建
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
    static const uint32 kMaxFileSize = 512 * 1024 * 1024;  // 日志文件最大为512M

    return !http_access_ofs_.is_open() || http_access_ofs_.tellp() > kMaxFileSize;
}

void FileLogger::CreateNewAccessRecordFile()
{
    http_access_ofs_.close();
    http_access_ofs_.open(boost::lexical_cast<std::string>(time(nullptr)).c_str());
}

}
