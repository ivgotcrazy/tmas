/*#############################################################################
 * 文件名   : record.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月13日
 * 文件描述 : DatabaseRecorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_RECORD
#define BROADINTER_RECORD

#include "tmas_typedef.hpp"
#include "connection.hpp"

namespace BroadInter
{

// TCP握手超时数据记录
struct TcpHsTimeoutRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK，记录时间
	uint32 timeout_value;	// 握手超时时间
};

// TCP连接超时数据记录
struct TcpConnTimeoutRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK，记录时间
	uint32 timeout_value;	// 连接超时时间
};

// TCP握手时延实时数据记录
struct TcpHsDelayRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK，记录时间
	uint32 hs_delay;		// 握手时延
};

// TCP连接异常关闭数据记录
struct TcpConnAbortRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK，记录时间
};

// HTTP响应时延数据记录
struct HttpRespDelayRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK, 记录时间
	std::string host;		// PK，Host
	std::string uri;		// PK，URI
	uint32 resp_delay;		// 响应时延
};

// HTTP下载速度数据记录
struct HttpDlSpeedRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK, 记录时间
	std::string host;		// PK，Host
	std::string uri;		// PK，URI
	uint32 dl_speed;		// 下载速度
};

/*
struct HttpAccessRecord
{
	ConnId conn_id;			// PK，连接标识
	uint32 record_time;		// PK，记录时间
	std::string method;		// Method
	std::string uri;		// URI
	uint32 status_code;		// 状态码
	std::string req_header;	// 原始请求头
	std::string resp_header;// 原始响应头
	uint32 resp_delay;		// 响应时延
	uint32 resp_elapsed;	// 接收响应消耗时间
	uint32 resp_size;		// 响应大小
};
*/

struct HttpAccessRecord
{
    uint64 access_time;  // 访问时间
    uint32 request_ip;  // 请求IP
    uint16 request_port;  // 请求端口号
    uint32 response_ip;  // 应答IP
    uint16 response_port;  // 应答端口号
    std::string uri;  // URI
    uint32 status_code;  // 状态码
    std::string request_header;  // 原始请求头
    std::string response_header;  // 原始响应头
    uint64 response_delay;  // 响应时延
    uint64 response_elapsed;  // 接收响应耗时
    uint32 response_size;  // 响应大小
    uint32 send_size;  // 请求报文大小
    uint32 receive_size;  // 响应报文大小
};

}

#endif
