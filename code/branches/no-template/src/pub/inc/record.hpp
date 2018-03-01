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

struct ConnRecord
{
	ConnId conn_id;	// PK，连接标识
	ptime begin;	// PK，连接开始时间(毫秒)

	uint32 duration; // 连接时长(毫秒)

	uint64 s2b_transfer; // 较小端向较大端传输字节数
	uint64 b2s_transfer; // 较大端向较小端传输字节数

	uint64 s2b_speed; // 较小端向较大端传输速度
	uint64 b2s_speed; // 较大端向较小端传输速度

	uint32 aver_resp_delay;	// 平均响应时延
	
	uint8 record_type; // 0: normal，1: 握手失败，2: 连接超时
};

struct HttpSessionRecord
{
	ConnId conn_id;	 // PK，连接标识
	std::string url; // PK，资源标识
	ptime begin;	 // PK，会话开始时间
	
	uint32 duration; // 会话时长

	uint32 resp_delay;	   // 响应时延
	uint32 download_speed; // 下载速度

	uint8 record_type; // 0: normal，1: 响应超时
};

}

#endif
