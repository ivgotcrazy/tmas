/*#############################################################################
 * 文件名   : pkt_parser_test.cpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年02月26日
 * 文件描述 : 报文解析单元测试
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <gtest/gtest.h>

#include "pkt_parser.hpp"
#include "message.hpp"

namespace BroadInter
{


/*------------------------------------------------------------------------------
 * 描述: 解析一个正常的TCP握手报文
 *----------------------------------------------------------------------------*/
TEST(ParsePktInfo, normal_tcp_pkt)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char tcp_packet[] = {
		0x90, 0x94, 0xe4, 0xa7, 0x5c, 0x86, // dst_mac
		0xe0, 0x69, 0x95, 0x16, 0x53, 0xea,	// src_mac

		0x81, 0x00, 0x12, 0x34,	// vlan tag 1
		0x81, 0x00, 0x56, 0x78, // vlan tag 2

		0x80, 0x00,				// type

		0x45,					// ver(4) + hdr_len(4)
		0x00,					// TOS
		0x00, 0x30,				// total_len
		0x2a, 0xcd,				// id
		0x40, 0x00,				// flag(3) + frag_off(13)
		0x40,					// TTL
		0x06,					// prot
		0x36, 0x67,				// checksum
		0xc0, 0xa8, 0x00, 0xd7, // src_ip
		0x65, 0xe2, 0xb2, 0x32,	// dst_ip

		0xf5, 0x99,				// src_port
		0x00, 0x50,				// dst_port
		0xff, 0x59, 0x0a, 0x18, // seq_num
		0x00, 0x00, 0x00, 0x00, // ack_num
		0x70, 0x02,				// hdr_len(4) + rsv(6) + flags(6)
		0x20, 0x00,				// wnd_size
		0xd9, 0xb6,				// checksum
		0x00, 0x00,				// urg ptr
		0x02, 0x04, 0x05, 0xb4, 0x01, 0x01, 0x04, 0x02};

#pragma GCC diagnostic warning "-Wnarrowing"

	PktMsgSP pkt_msg(new PktMsg);

	pkt_msg->pkt.buf = tcp_packet;
	pkt_msg->pkt.len = sizeof(tcp_packet);

	//--------------------------------------------------------------------------
	//--- ETH层报文解析
	//--------------------------------------------------------------------------
	ASSERT_TRUE(ParsePktEthInfo(pkt_msg));

	ASSERT_EQ((char)0x90, pkt_msg->l2_pkt_info.dst_mac[0]);
	ASSERT_EQ((char)0x94, pkt_msg->l2_pkt_info.dst_mac[1]);
	ASSERT_EQ((char)0xe4, pkt_msg->l2_pkt_info.dst_mac[2]);
	ASSERT_EQ((char)0xa7, pkt_msg->l2_pkt_info.dst_mac[3]);
	ASSERT_EQ((char)0x5c, pkt_msg->l2_pkt_info.dst_mac[4]);
	ASSERT_EQ((char)0x86, pkt_msg->l2_pkt_info.dst_mac[5]);

	ASSERT_EQ((char)0xe0, pkt_msg->l2_pkt_info.src_mac[0]);
	ASSERT_EQ((char)0x69, pkt_msg->l2_pkt_info.src_mac[1]);
	ASSERT_EQ((char)0x95, pkt_msg->l2_pkt_info.src_mac[2]);
	ASSERT_EQ((char)0x16, pkt_msg->l2_pkt_info.src_mac[3]);
	ASSERT_EQ((char)0x53, pkt_msg->l2_pkt_info.src_mac[4]);
	ASSERT_EQ((char)0xea, pkt_msg->l2_pkt_info.src_mac[5]);

	ASSERT_EQ(0x8000, pkt_msg->l2_pkt_info.l3_prot);

	ASSERT_EQ(48, pkt_msg->l2_pkt_info.l2_data_len);

	//--------------------------------------------------------------------------
	//--- IP层报文解析
	//--------------------------------------------------------------------------
	ASSERT_TRUE(ParsePktIpInfo(true, pkt_msg));

	ASSERT_EQ(4, pkt_msg->l3_pkt_info.ip_ver);
	ASSERT_EQ(6, pkt_msg->l3_pkt_info.l4_prot);

	ASSERT_EQ(0xc0a800d7, pkt_msg->l3_pkt_info.src_ip);
	ASSERT_EQ(0x65e2b232, pkt_msg->l3_pkt_info.dst_ip);

	ASSERT_EQ(PKT_DIR_B2S, pkt_msg->l3_pkt_info.direction);

	ASSERT_EQ(0, pkt_msg->l3_pkt_info.mf_flag);
	ASSERT_EQ(0, pkt_msg->l3_pkt_info.frag_offset);
	ASSERT_EQ(0x2acd, pkt_msg->l3_pkt_info.pkt_id);

	ASSERT_EQ(28, pkt_msg->l3_pkt_info.l3_data_len);

	//--------------------------------------------------------------------------
	//--- TCP层报文解析
	//--------------------------------------------------------------------------
	ASSERT_TRUE(ParsePktTcpInfo(pkt_msg));

	ASSERT_EQ(0xf599, pkt_msg->l4_pkt_info.src_port);
	ASSERT_EQ(0x0050, pkt_msg->l4_pkt_info.dst_port);

	ASSERT_EQ(0xff590a18, pkt_msg->l4_pkt_info.seq_num);
	ASSERT_EQ(0x0, pkt_msg->l4_pkt_info.ack_num);

	ASSERT_TRUE(pkt_msg->l4_pkt_info.syn_flag);
	ASSERT_FALSE(pkt_msg->l4_pkt_info.ack_flag);
	ASSERT_FALSE(pkt_msg->l4_pkt_info.fin_flag);
	ASSERT_FALSE(pkt_msg->l4_pkt_info.rst_flag);

	ASSERT_EQ(0, pkt_msg->l4_pkt_info.l4_data_len);

}

/*------------------------------------------------------------------------------
 * 描述: 非正常报文，只包含ETH头和IP头
 *----------------------------------------------------------------------------*/
TEST(ParsePktInfo, too_short_pkt)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char short_packet[] = {
		0x90, 0x94, 0xe4, 0xa7, 0x5c, 0x86, // dst_mac
		0xe0, 0x69, 0x95, 0x16, 0x53, 0xea,	// src_mac
		0x80, 0x00,				// type

		0x45,					// ver(4) + hdr_len(4)
		0x00,					// TOS
		0x00, 0x30,				// total_len
		0x2a, 0xcd,				// id
		0x40, 0x00,				// flag(3) + frag_off(13)
		0x40,					// TTL
		0x06,					// prot
		0x36, 0x67,				// checksum
		0xc0, 0xa8, 0x00, 0xd7, // src_ip
		0x65, 0xe2, 0xb2, 0x32,	// dst_ip
	};

#pragma GCC diagnostic warning "-Wnarrowing"

	PktMsgSP pkt_msg(new PktMsg);

	pkt_msg->pkt.buf = short_packet;
	pkt_msg->pkt.len = sizeof(short_packet);

	ASSERT_FALSE(ParsePktEthInfo(pkt_msg));
}

/*------------------------------------------------------------------------------
 * 描述: 非正常报文——ip头长度为16字节，小于最小IP报文头长度
 *----------------------------------------------------------------------------*/
TEST(ParsePktInfo, invalid_ip_hdr_len)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char ip_packet[] = {
		0x90, 0x94, 0xe4, 0xa7, 0x5c, 0x86, // dst_mac
		0xe0, 0x69, 0x95, 0x16, 0x53, 0xea,	// src_mac
		0x80, 0x00,				// type

		0x44,					// ver(4) + hdr_len(4)  <==!!!
		0x00,					// TOS
		0x00, 0x30,				// total_len
		0x2a, 0xcd,				// id
		0x40, 0x00,				// flag(3) + frag_off(13)
		0x40,					// TTL
		0x06,					// prot
		0x36, 0x67,				// checksum
		0xc0, 0xa8, 0x00, 0xd7, // src_ip
		0x65, 0xe2, 0xb2, 0x32,	// dst_ip

		0xf5, 0x99,				// src_port
		0x00, 0x50,				// dst_port
		0xff, 0x59, 0x0a, 0x18, // seq_num
		0x00, 0x00, 0x00, 0x00, // ack_num
		0x70, 0x02,				// hdr_len(4) + rsv(6) + flags(6)
		0x20, 0x00,				// wnd_size
		0xd9, 0xb6,				// checksum
		0x00, 0x00,				// urg ptr
		0x02, 0x04, 0x05, 0xb4, 0x01, 0x01, 0x04, 0x02 
	};

#pragma GCC diagnostic warning "-Wnarrowing"

	PktMsgSP pkt_msg(new PktMsg);

	pkt_msg->pkt.buf = ip_packet;
	pkt_msg->pkt.len = sizeof(ip_packet);

	ASSERT_TRUE(ParsePktEthInfo(pkt_msg));

	ASSERT_FALSE(ParsePktIpInfo(true, pkt_msg));
}

/*------------------------------------------------------------------------------
 * 描述: 非正常报文——ip头部校验和校验失败
 *----------------------------------------------------------------------------*/
TEST(ParsePktInfo, invalid_ip_checksum)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char ip_packet[] = {
		0x90, 0x94, 0xe4, 0xa7, 0x5c, 0x86, // dst_mac
		0xe0, 0x69, 0x95, 0x16, 0x53, 0xea,	// src_mac
		0x80, 0x00,				// type

		0x45,					// ver(4) + hdr_len(4)
		0x00,					// TOS
		0x00, 0x20,				// total_len	<==!!!
		0x2a, 0xcd,				// id
		0x40, 0x00,				// flag(3) + frag_off(13)
		0x40,					// TTL
		0x06,					// prot
		0x36, 0x67,				// checksum		<==!!!
		0xc0, 0xa8, 0x00, 0xd7, // src_ip
		0x65, 0xe2, 0xb2, 0x32,	// dst_ip

		0xf5, 0x99,				// src_port
		0x00, 0x50,				// dst_port
		0xff, 0x59, 0x0a, 0x18, // seq_num
		0x00, 0x00, 0x00, 0x00, // ack_num
		0x70, 0x02,				// hdr_len(4) + rsv(6) + flags(6)
		0x20, 0x00,				// wnd_size
		0xd9, 0xb6,				// checksum
		0x00, 0x00,				// urg ptr
		0x02, 0x04, 0x05, 0xb4, 0x01, 0x01, 0x04, 0x02 
	};

#pragma GCC diagnostic warning "-Wnarrowing"

	PktMsgSP pkt_msg(new PktMsg);

	pkt_msg->pkt.buf = ip_packet;
	pkt_msg->pkt.len = sizeof(ip_packet);

	ASSERT_TRUE(ParsePktEthInfo(pkt_msg));

	ASSERT_FALSE(ParsePktIpInfo(true, pkt_msg));
}

/*------------------------------------------------------------------------------
 * 描述: 非正常报文——IP总长度大于ETH层计算的长度
 *----------------------------------------------------------------------------*/
TEST(ParsePktInfo, too_big_ip_total_len)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char ip_packet[] = {
		0x90, 0x94, 0xe4, 0xa7, 0x5c, 0x86, // dst_mac
		0xe0, 0x69, 0x95, 0x16, 0x53, 0xea,	// src_mac
		0x80, 0x00,				// type

		0x45,					// ver(4) + hdr_len(4)
		0x00,					// TOS
		0x00, 0x40,				// total_len	<==!!!
		0x2a, 0xcd,				// id
		0x40, 0x00,				// flag(3) + frag_off(13)
		0x40,					// TTL
		0x06,					// prot
		0x36, 0x57,				// checksum
		0xc0, 0xa8, 0x00, 0xd7, // src_ip
		0x65, 0xe2, 0xb2, 0x32,	// dst_ip

		0xf5, 0x99,				// src_port
		0x00, 0x50,				// dst_port
		0xff, 0x59, 0x0a, 0x18, // seq_num
		0x00, 0x00, 0x00, 0x00, // ack_num
		0x70, 0x02,				// hdr_len(4) + rsv(6) + flags(6)
		0x20, 0x00,				// wnd_size
		0xd9, 0xb6,				// checksum
		0x00, 0x00,				// urg ptr
		0x02, 0x04, 0x05, 0xb4, 0x01, 0x01, 0x04, 0x02 
	};

#pragma GCC diagnostic warning "-Wnarrowing"

	PktMsgSP pkt_msg(new PktMsg);

	pkt_msg->pkt.buf = ip_packet;
	pkt_msg->pkt.len = sizeof(ip_packet);

	ASSERT_TRUE(ParsePktEthInfo(pkt_msg));

	ASSERT_FALSE(ParsePktIpInfo(true, pkt_msg));
}

/*------------------------------------------------------------------------------
 * 描述: 非正常报文——IP数据长度小于TCP头长度
 *----------------------------------------------------------------------------*/
TEST(ParsePktInfo, too_small_ip_total_len)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char ip_packet[] = {
		0x90, 0x94, 0xe4, 0xa7, 0x5c, 0x86, // dst_mac
		0xe0, 0x69, 0x95, 0x16, 0x53, 0xea,	// src_mac
		0x80, 0x00,				// type

		0x45,					// ver(4) + hdr_len(4)
		0x00,					// TOS
		0x00, 0x20,				// total_len	<==!!!
		0x2a, 0xcd,				// id
		0x40, 0x00,				// flag(3) + frag_off(13)
		0x40,					// TTL
		0x06,					// prot
		0x36, 0x77,				// checksum
		0xc0, 0xa8, 0x00, 0xd7, // src_ip
		0x65, 0xe2, 0xb2, 0x32,	// dst_ip

		0xf5, 0x99,				// src_port
		0x00, 0x50,				// dst_port
		0xff, 0x59, 0x0a, 0x18, // seq_num
		0x00, 0x00, 0x00, 0x00, // ack_num
		0x70, 0x02,				// hdr_len(4) + rsv(6) + flags(6)
		0x20, 0x00,				// wnd_size
		0xd9, 0xb6,				// checksum
		0x00, 0x00,				// urg ptr
		0x02, 0x04, 0x05, 0xb4, 0x01, 0x01, 0x04, 0x02 
	};

#pragma GCC diagnostic warning "-Wnarrowing"

	PktMsgSP pkt_msg(new PktMsg);

	pkt_msg->pkt.buf = ip_packet;
	pkt_msg->pkt.len = sizeof(ip_packet);

	ASSERT_TRUE(ParsePktEthInfo(pkt_msg));

	ASSERT_TRUE(ParsePktIpInfo(true, pkt_msg));

	ASSERT_FALSE(ParsePktTcpInfo(pkt_msg));
}

}