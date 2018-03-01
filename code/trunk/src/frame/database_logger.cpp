/*#############################################################################
 * �ļ���   : database_logger.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : DatabaseLogger��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "database_logger.hpp"
#include "tmas_config_parser.hpp"

namespace BroadInter
{

#define TCP_HS_TIMEOUT_TBL		"tcp_hs_timeout_tbl"
#define TCP_HS_DELAY_TBL		"tcp_hs_delay_tbl"
#define TCP_CONN_TIMEOUT_TBL	"tcp_conn_timeout_tbl"
#define TCP_CONN_ABORT_TBL		"tcp_conn_abort_tbl"
#define HTTP_RESP_DELAY_TBL		"http_resp_delay_tbl"
#define HTTP_DL_SPEED_TBL		"http_dl_speed_tbl"

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
DatabaseLogger::~DatabaseLogger()
{
	mysql_close(&mysql_handle_);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool DatabaseLogger::Init()
{
	// ��ʼ�����Ӵ���
	if (!mysql_init(&mysql_handle_))
	{
		LOG(ERROR) << "Fail to init mysql";
		return false;
	}

	if (!ConnectToDatabase()) return false;

	if (!IsAllTablesCreated()) return false;

	DLOG(INFO) << "Initialize mysql successfully";

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������ݿ�
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool DatabaseLogger::ConnectToDatabase()
{
	// �������ļ���ȡ���ݿ����Ӳ���
	
	std::string db_host;
	GET_TMAS_CONFIG_STR("global.database.host", db_host);

	uint32 db_port;
	GET_TMAS_CONFIG_INT("global.database.port", db_port);

	std::string db_user;
	GET_TMAS_CONFIG_STR("global.database.user", db_user);

	std::string db_pw;
	GET_TMAS_CONFIG_STR("global.database.password", db_pw);

	std::string db_name;
	GET_TMAS_CONFIG_STR("global.database.name", db_name);
	
	// ���ӵ����ݿ�
	if (!mysql_real_connect(&mysql_handle_,		// MYSQL�ṹ
							db_host.c_str(),	// ��������IP��ַ
							db_user.c_str(),	// MySql��½ID
							db_pw.c_str(),		// �û�����
							db_name.c_str(),	// ���ݿ�����
							db_port,			// TCP/IP���ӵĶ˿ں�
							NULL,				// ʹ�õ��׽��ֻ������ܵ�
							0))					// client_flag
	{
		LOG(ERROR) << "Fail to connect mysql | " << mysql_error(&mysql_handle_);
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ����е����ݿ���Ѿ�����
 * ��  ��: 
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool DatabaseLogger::IsAllTablesCreated()
{
	if (!IsTableCreated(TCP_HS_TIMEOUT_TBL))
	{
		LOG(ERROR) << "Can not find table " << TCP_HS_TIMEOUT_TBL;
		return false;
	}

	if (!IsTableCreated(TCP_HS_DELAY_TBL))
	{
		LOG(ERROR) << "Can not find table " << TCP_HS_DELAY_TBL;
		return false;
	}

	if (!IsTableCreated(TCP_CONN_TIMEOUT_TBL))
	{
		LOG(ERROR) << "Can not find table " << TCP_CONN_TIMEOUT_TBL;
		return false;
	}

	if (!IsTableCreated(TCP_CONN_ABORT_TBL))
	{
		LOG(ERROR) << "Can not find table " << TCP_CONN_ABORT_TBL;
		return false;
	}

	if (!IsTableCreated(HTTP_RESP_DELAY_TBL))
	{
		LOG(ERROR) << "Can not find table " << HTTP_RESP_DELAY_TBL;
		return false;
	}

	if (!IsTableCreated(HTTP_DL_SPEED_TBL))
	{
		LOG(ERROR) << "Can not find table " << HTTP_DL_SPEED_TBL;
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ָ�����ݿ���Ƿ��Ѿ�����
 * ��  ��: [in] table_name ���ݿ����
 * ����ֵ: ��/��
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool DatabaseLogger::IsTableCreated(const std::string& table_name)
{
	std::string query_str = "show tables like '" + table_name + "'";
	if (mysql_query(&mysql_handle_, query_str.c_str()))
	{
		LOG(ERROR) << "Fail to query table from database";
		return false;
	}

	MYSQL_RES* result = mysql_store_result(&mysql_handle_);
	if (!result)
	{
		LOG(ERROR) << "Fail to store result";
		return false;
	}

	if (1 != mysql_num_rows(result))
	{
		LOG(ERROR) << "Unexpected table number";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogTcpHsTimeoutRecord(const TcpHsTimeoutRecord& record)
{
	char query_str[256];

	snprintf(query_str, 256,
		"INSERT INTO tcp_hs_timeout_tbl VALUES (%d, %d, %d, %d, %d, %d)", 
		record.record_time, 
		record.conn_id.smaller_ip,
		record.conn_id.smaller_port,
		record.conn_id.bigger_ip,
		record.conn_id.bigger_port,
		record.timeout_value);

	if (mysql_query(&mysql_handle_, query_str))
	{
		LOG(ERROR) << "Fail to insert record to tcp_hs_timeout_tbl";
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogTcpHsDelayRecord(const TcpHsDelayRecord& record)
{
	char query_str[256];

	snprintf(query_str, 256,
		"INSERT INTO tcp_hs_delay_tbl VALUES (%d, %d, %d, %d, %d, %d)",
		record.record_time,
		record.conn_id.smaller_ip,
		record.conn_id.smaller_port,
		record.conn_id.bigger_ip,
		record.conn_id.bigger_port,
		record.hs_delay);

	if (mysql_query(&mysql_handle_, query_str))
	{
		LOG(ERROR) << "Fail to insert record to tcp_hs_delay_tbl";
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record)
{
	char query_str[256];

	snprintf(query_str, 256,
		"INSERT INTO tcp_conn_timeout_tbl VALUES (%d, %d, %d, %d, %d, %d)",
		record.record_time,
		record.conn_id.smaller_ip,
		record.conn_id.smaller_port,
		record.conn_id.bigger_ip,
		record.conn_id.bigger_port,
		record.timeout_value);

	if (mysql_query(&mysql_handle_, query_str))
	{
		LOG(ERROR) << "Fail to insert record to tcp_hs_delay_stat_tbl";
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogTcpConnAbortRecord(const TcpConnAbortRecord& record)
{
	char* query_str = new char[256];

	snprintf(query_str, 256,
		"INSERT INTO tcp_conn_abort_tbl VALUES (%d, %d, %d, %d, %d)",
		record.record_time,
		record.conn_id.smaller_ip,
		record.conn_id.smaller_port,
		record.conn_id.bigger_ip,
		record.conn_id.bigger_port);

	if (mysql_query(&mysql_handle_, query_str))
	{
		LOG(ERROR) << "Fail to insert record to http_host_tbl";
	}

	delete[] query_str;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogHttpRespDelayRecord(const HttpRespDelayRecord& record)
{
	uint32 length = record.host.size() + record.uri.size() + 256;

	char *query_str = new char[length];

	snprintf(query_str, length,
		"INSERT INTO http_resp_delay_tbl VALUES (%d, %d, %d, %d, %d, %s, %s, %d)",
		record.record_time,
		record.conn_id.smaller_ip,
		record.conn_id.smaller_port,
		record.conn_id.bigger_ip,
		record.conn_id.bigger_port,
		record.host.c_str(),
		record.uri.c_str(),
		record.resp_delay);

	if (mysql_query(&mysql_handle_, query_str))
	{
		LOG(ERROR) << "Fail to insert record to http_host_stat_tbl";
	}

	delete[] query_str;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼TCP���ֳ�ʱ��¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��07��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogHttpDlSpeedRecord(const HttpDlSpeedRecord& record)
{
	uint32 length = record.host.size() + record.uri.size() + 256;

	char *query_str = new char[length];

	snprintf(query_str, length,
		"INSERT INTO http_dl_speed_tbl VALUES (%d, %d, %d, %d, %d, %s, %s, %d)",
		record.record_time,
		record.conn_id.smaller_ip,
		record.conn_id.smaller_port,
		record.conn_id.bigger_ip,
		record.conn_id.bigger_port,
		record.host.c_str(),
		record.uri.c_str(),
		record.dl_speed);

	if (mysql_query(&mysql_handle_, query_str))
	{
		LOG(ERROR) << "Fail to insert record to http_uri_tbl";
	}

	delete[] query_str;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼HTTP���ʼ�¼
 * ��  ��: [in] record ���ݼ�¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��05��12��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseLogger::LogHttpAccessRecord(const HttpAccessRecord& record)
{

}

}