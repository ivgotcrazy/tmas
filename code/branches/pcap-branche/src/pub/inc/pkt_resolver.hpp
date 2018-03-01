/*#############################################################################
 * 文件名   : pkt_resolver.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月03日
 * 文件描述 : PktResolver类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_RESOLVER
#define BROADINTER_PKT_RESOLVER

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "tmas_typedef.hpp"

namespace BroadInter
{

namespace PR // packet resolver
{

// IP伪首部，用于计算校验和
#pragma pack(1)
	struct PseudoHeader
	{
		uint32 source_ip = 0;  // 源IP地址
		uint32 dest_ip	 = 0;  // 目的IP地址
		uint8  user      = 0;  // 未知
		uint8  protocol  = 0;  // 传输层协议
		uint16 length    = 0;  // IP层数据长度
	};
#pragma pack()

//--- IP层

inline uint32 GetSourceIpN(char* ip_hdr);	// 获取IP层源地址（网络字节顺序）
inline uint32 GetSourceIpH(char* ip_hdr);	// 获取IP层源地址（主机字节顺序）
inline uint32 GetDestIpN(char* ip_hdr);		// 获取IP层目的地址（网络字节顺序）
inline uint32 GetDestIpH(char* ip_hdr);		// 获取IP层目的地址（主机字节顺序）

inline uint16 GetFragPktId(char* ip_hdr);	// 获取分片报文标识
inline uint16 GetFragOffset(char* ip_hdr);	// 获取分片偏移
inline bool GetMfFlag(char* ip_hdr);		// 获取分片标识

inline uint16 GetIpHdrLen(char* ip_hdr);	// 获取IP头部长度
inline uint16 GetIpDataLen(char* ip_hdr);	// 获取IP数据长度
inline uint16 GetIpTotalLen(char* ip_hdr);	// 获取IP报文总长度

inline uint8 GetIpHdrProt(char* ip_hdr);	// 获取IP层承载协议
inline uint8 GetIpVersion(char* ip_hdr);	// 获取IP协议版本

inline bool CheckIpHeaderCrc(char* ip_hdr, uint16 header_len);	// IP头部的CRC校验

inline void SetIpTotalLen(char* ip_hdr, uint16 total_len);	// 设置IP数据总长度
inline void SetMfFlag(char* ip_hdr, bool flag);
inline void SetFragOffset(char* ip_hdr, uint16 offset);
inline void SetFragPktId(char* ip_hdr, uint16 pkt_id);

//--- TCP

inline uint16 GetTcpSrcPortN(char* l4_hdr);
inline uint16 GetTcpSrcPortH(char* l4_hdr);
inline uint16 GetTcpDstPortN(char* l4_hdr);
inline uint16 GetTcpDstPortH(char* l4_hdr);

inline bool IsTcpAckSet(char* l4_hdr);	// ACK标志位
inline bool IsTcpSynSet(char* l4_hdr);	// SYN标志位
inline bool IsTcpFinSet(char* l4_hdr);	// FIN标志位
inline bool IsTcpRstSet(char* l4_hdr);	// RST标志位

inline uint32 GetTcpSeqNum(char* l4_hdr);	// 顺序号
inline uint32 GetTcpAckNum(char* l4_hdr);	// 确认号

inline uint16 GetTcpHdrLen(char* l4_hdr);	// TCP头部长度

//--- UDP

inline uint16 GetUdpSrcPortN(char* l4_hdr);
inline uint16 GetUdpSrcPortH(char* l4_hdr);
inline uint16 GetUdpDstPortN(char* l4_hdr);
inline uint16 GetUdpDstPortH(char* l4_hdr);

//--- 其他

PseudoHeader GetPseudoHeader();  // 获取伪首部

} // namespace PR

} // namespace BroadInter

#include "pkt_resolver_inl.hpp"

#endif

