/*#############################################################################
 * �ļ���   : record.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��13��
 * �ļ����� : DatabaseRecorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_RECORD
#define BROADINTER_RECORD

#include "tmas_typedef.hpp"
#include "connection.hpp"

namespace BroadInter
{

// TCP���ֳ�ʱ���ݼ�¼
struct TcpHsTimeoutRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK����¼ʱ��
	uint32 timeout_value;	// ���ֳ�ʱʱ��
};

// TCP���ӳ�ʱ���ݼ�¼
struct TcpConnTimeoutRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK����¼ʱ��
	uint32 timeout_value;	// ���ӳ�ʱʱ��
};

// TCP����ʱ��ʵʱ���ݼ�¼
struct TcpHsDelayRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK����¼ʱ��
	uint32 hs_delay;		// ����ʱ��
};

// TCP�����쳣�ر����ݼ�¼
struct TcpConnAbortRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK����¼ʱ��
};

// HTTP��Ӧʱ�����ݼ�¼
struct HttpRespDelayRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK, ��¼ʱ��
	std::string host;		// PK��Host
	std::string uri;		// PK��URI
	uint32 resp_delay;		// ��Ӧʱ��
};

// HTTP�����ٶ����ݼ�¼
struct HttpDlSpeedRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK, ��¼ʱ��
	std::string host;		// PK��Host
	std::string uri;		// PK��URI
	uint32 dl_speed;		// �����ٶ�
};

/*
struct HttpAccessRecord
{
	ConnId conn_id;			// PK�����ӱ�ʶ
	uint32 record_time;		// PK����¼ʱ��
	std::string method;		// Method
	std::string uri;		// URI
	uint32 status_code;		// ״̬��
	std::string req_header;	// ԭʼ����ͷ
	std::string resp_header;// ԭʼ��Ӧͷ
	uint32 resp_delay;		// ��Ӧʱ��
	uint32 resp_elapsed;	// ������Ӧ����ʱ��
	uint32 resp_size;		// ��Ӧ��С
};
*/

struct HttpAccessRecord
{
    uint64 access_time;  // ����ʱ��
    uint32 request_ip;  // ����IP
    uint16 request_port;  // ����˿ں�
    uint32 response_ip;  // Ӧ��IP
    uint16 response_port;  // Ӧ��˿ں�
    std::string uri;  // URI
    uint32 status_code;  // ״̬��
    std::string request_header;  // ԭʼ����ͷ
    std::string response_header;  // ԭʼ��Ӧͷ
    uint64 response_delay;  // ��Ӧʱ��
    uint64 response_elapsed;  // ������Ӧ��ʱ
    uint32 response_size;  // ��Ӧ��С
    uint32 send_size;  // �����Ĵ�С
    uint32 receive_size;  // ��Ӧ���Ĵ�С
};

}

#endif
