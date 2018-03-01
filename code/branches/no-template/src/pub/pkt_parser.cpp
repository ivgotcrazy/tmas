/*#############################################################################
 * 文件名   : pkt_parser.cpp
 * 创建人   : teck_zhou
 * 创建时间 : 2014年02月22日
 * 文件描述 : 报文各层信息解析
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "pkt_parser.hpp"
#include "tmas_io.hpp"
#include "pkt_resolver.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 解析ETH层信息
 * 参  数: [in][out] pkt_msg 报文消息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月22日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool ParsePktEthInfo(PktMsgSP& pkt_msg)
{
	if ((pkt_msg->pkt.len < MIN_ETH_PKT_LEN) 
		|| (pkt_msg->pkt.len > MAX_ETH_PKT_LEN))
	{
		DLOG(WARNING) << "Invalid ethernet packet length | " << pkt_msg->pkt.len;
		return false;
	}

	// 拷贝目的MAC和源MAC
	std::memcpy(pkt_msg->l2_pkt_info.dst_mac, pkt_msg->pkt.buf, MAC_LEN);
	std::memcpy(pkt_msg->l2_pkt_info.src_mac, pkt_msg->pkt.buf + MAC_LEN, MAC_LEN);

	// 偏移到TYPE字段，开始解析VLAN和上层协议
	char* start = const_cast<char*>(pkt_msg->pkt.buf) + MAC_LEN * 2;
	char* end = start + pkt_msg->pkt.len;

	uint16 prot = 0;
	uint16 resolved_data_len = 0;

	while (start < end)
	{
		prot = IO::read_uint16(start);
		resolved_data_len += 2;

		if (prot != TMAS_ETH_TYPE_8021Q) break; // ETH层只解析802.1q

		pkt_msg->l2_pkt_info.vlan_num++;
		start += 2; // VLAN TAG为4个字节，前面read时已经往后移了2个字节
		resolved_data_len += 2;
	}

	if (start >= end) return false;

	pkt_msg->l2_pkt_info.l3_prot = prot;
	ETH_HDR_LEN(pkt_msg) = MAC_LEN * 2 + resolved_data_len;
	L2_DATA_LEN(pkt_msg) = pkt_msg->pkt.len - ETH_HDR_LEN(pkt_msg);
	L3_HDR(pkt_msg) = const_cast<char*>(pkt_msg->pkt.buf) + ETH_HDR_LEN(pkt_msg);

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析IP层信息
 * 参  数: [in] enable_checksum 是否做IP头checksum
 *         [in][out] pkt_msg 报文消息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月22日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool ParsePktIpInfo(bool enable_checksum, PktMsgSP& pkt_msg)
{
	if (PR::GetIpHdrLen(L3_HDR(pkt_msg)) < IP_HEADER_MIN_LEN)
	{
		LOG(ERROR) << "Invalid ip header length: " 
			       << PR::GetIpHdrLen(L3_HDR(pkt_msg));
		return false;
	}

	// 校验从IP头获取的总长度与ETH计算的长度的关系
	if (pkt_msg->l2_pkt_info.l2_data_len < PR::GetIpTotalLen(L3_HDR(pkt_msg)))
	{
		LOG(ERROR) << "Invalid IP total length | l2: " 
			       << pkt_msg->l2_pkt_info.l2_data_len
			       << " l3: " << PR::GetIpTotalLen(L3_HDR(pkt_msg));
		return false;
	}

	if (enable_checksum // IP头部的CRC校验
		&& !PR::CheckIpHeaderCrc(L3_HDR(pkt_msg), PR::GetIpHdrLen(L3_HDR(pkt_msg))))
	{	
		LOG(ERROR) << "Fail to check IP header CRC";
		return false;
	}

	// TODO: 这里还需要做下IP选项的校验，留待后续增加

	// 承载数据长度
	L3_DATA_LEN(pkt_msg) = PR::GetIpDataLen(L3_HDR(pkt_msg));

	// 源IP和目的IP
	SRC_IP(pkt_msg) = PR::GetSourceIpH(L3_HDR(pkt_msg));
	DST_IP(pkt_msg) = PR::GetDestIpH(L3_HDR(pkt_msg));

	// 分片信息
	pkt_msg->l3_pkt_info.mf_flag = PR::GetMfFlag(L3_HDR(pkt_msg));
	pkt_msg->l3_pkt_info.frag_offset = PR::GetFragOffset(L3_HDR(pkt_msg));
	pkt_msg->l3_pkt_info.pkt_id = PR::GetFragPktId(L3_HDR(pkt_msg));

	// 定位L4数据起始位置
	pkt_msg->l4_pkt_info.l4_hdr = pkt_msg->l3_pkt_info.l3_hdr + 
		PR::GetIpHdrLen(L3_HDR(pkt_msg));

	// 获取L4协议类型
	pkt_msg->l3_pkt_info.l4_prot = PR::GetIpHdrProt(L3_HDR(pkt_msg));

	// IP协议版本
	pkt_msg->l3_pkt_info.ip_ver = PR::GetIpVersion(L3_HDR(pkt_msg));

	// 报文方向
	pkt_msg->l3_pkt_info.direction = 
		(SRC_IP(pkt_msg) < DST_IP(pkt_msg)) ? PKT_DIR_S2B : PKT_DIR_B2S;

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 解析TCP层信息
 * 参  数: [in][out] pkt_msg 报文消息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月22日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool ParsePktTcpInfo(PktMsgSP& pkt_msg)
{
	if (PR::GetTcpHdrLen(L4_HDR(pkt_msg)) < 20)
	{
		DLOG(WARNING) << "Invalid tcp header length: "
			         << PR::GetTcpHdrLen(L4_HDR(pkt_msg));
		return false;
	}

	if (L3_DATA_LEN(pkt_msg) < PR::GetTcpHdrLen(L4_HDR(pkt_msg)))
	{
		DLOG(WARNING) << "L3 payload length less than min tcp header length |"
			         << " l3_data_len: " << L3_DATA_LEN(pkt_msg) 
					 << " tcp_hdr_len: " << PR::GetTcpHdrLen(L4_HDR(pkt_msg));
		return false;
	}

	pkt_msg->l4_pkt_info.src_port = PR::GetTcpSrcPortH(pkt_msg->l4_pkt_info.l4_hdr);
	pkt_msg->l4_pkt_info.dst_port = PR::GetTcpDstPortH(pkt_msg->l4_pkt_info.l4_hdr);

	pkt_msg->l4_pkt_info.conn_id = ConnId(IPPROTO_TCP,
		                                  pkt_msg->l3_pkt_info.src_ip,
										  pkt_msg->l4_pkt_info.src_port,
										  pkt_msg->l3_pkt_info.dst_ip,
										  pkt_msg->l4_pkt_info.dst_port);

	pkt_msg->l4_pkt_info.seq_num = PR::GetTcpSeqNum(L4_HDR(pkt_msg));
	pkt_msg->l4_pkt_info.ack_num = PR::GetTcpAckNum(L4_HDR(pkt_msg));

	pkt_msg->l4_pkt_info.ack_flag = PR::IsTcpAckSet(L4_HDR(pkt_msg));
	pkt_msg->l4_pkt_info.syn_flag = PR::IsTcpSynSet(L4_HDR(pkt_msg));
	pkt_msg->l4_pkt_info.fin_flag = PR::IsTcpFinSet(L4_HDR(pkt_msg));
	pkt_msg->l4_pkt_info.rst_flag = PR::IsTcpRstSet(L4_HDR(pkt_msg));

	L4_DATA_LEN(pkt_msg) = L3_DATA_LEN(pkt_msg) - PR::GetTcpHdrLen(L4_HDR(pkt_msg));

	L7_DATA(pkt_msg) = L4_HDR(pkt_msg) + PR::GetTcpHdrLen(L4_HDR(pkt_msg));

	return true;
}

}