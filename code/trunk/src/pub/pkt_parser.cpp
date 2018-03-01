/*#############################################################################
 * �ļ���   : pkt_parser.cpp
 * ������   : teck_zhou
 * ����ʱ�� : 2014��02��22��
 * �ļ����� : ���ĸ�����Ϣ����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "pkt_parser.hpp"
#include "tmas_io.hpp"
#include "pkt_resolver.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ����ETH����Ϣ
 * ��  ��: [in][out] pkt_msg ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��22��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ParsePktEthInfo(PktMsg* pkt_msg)
{
	if ((pkt_msg->pkt.len < MIN_ETH_PKT_LEN) 
		|| (pkt_msg->pkt.len > MAX_ETH_PKT_LEN))
	{
		DLOG(WARNING) << "Invalid ethernet packet length | " << pkt_msg->pkt.len;
		return false;
	}

	// ����Ŀ��MAC��ԴMAC
	std::memcpy(pkt_msg->l2_pkt_info.dst_mac, pkt_msg->pkt.buf, MAC_LEN);
	std::memcpy(pkt_msg->l2_pkt_info.src_mac, pkt_msg->pkt.buf + MAC_LEN, MAC_LEN);

	// ƫ�Ƶ�TYPE�ֶΣ���ʼ����VLAN���ϲ�Э��
	char* start = pkt_msg->pkt.buf + MAC_LEN * 2;
	char* end = start + pkt_msg->pkt.len;

	uint16 prot = 0;
	uint16 resolved_data_len = 0;

	while (start < end)
	{
		prot = IO::read_uint16(start);
		resolved_data_len += 2;

		// ETH��ֻ����802.1q
		if (prot != TMAS_ETH_TYPE_8021Q) break; 
			
		pkt_msg->l2_pkt_info.vlan_num++;
		start += 2; // VLAN TAGΪ4���ֽڣ�ǰ��readʱ�Ѿ���������2���ֽ�
		resolved_data_len += 2;
	}

	if (start >= end) return false;

	pkt_msg->l2_pkt_info.l3_prot = prot;
	ETH_HDR_LEN(pkt_msg) = MAC_LEN * 2 + resolved_data_len;
	L2_DATA_LEN(pkt_msg) = pkt_msg->pkt.len - ETH_HDR_LEN(pkt_msg);
	L3_HDR(pkt_msg) = pkt_msg->pkt.buf + ETH_HDR_LEN(pkt_msg);

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����IP����Ϣ
 * ��  ��: [in] enable_checksum �Ƿ���IPͷchecksum
 *         [in][out] pkt_msg ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��22��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ParsePktIpInfo(bool enable_checksum, PktMsg* pkt_msg)
{
	if (PR::GetIpHdrLen(L3_HDR(pkt_msg)) < IP_HEADER_MIN_LEN)
	{
		LOG(ERROR) << "Invalid ip header length: " 
			       << PR::GetIpHdrLen(L3_HDR(pkt_msg));
		return false;
	}

	// У���IPͷ��ȡ���ܳ�����ETH����ĳ��ȵĹ�ϵ
	// TODO: ��ץ�����������������ETHͷ�����IP���ĳ��ȴ��ڴ�IPͷЯ���ı���
	// ���ȣ���ֵΪ6���ֽڣ����ɿ�����֡β����ȷ����
	if (pkt_msg->l2_pkt_info.l2_data_len < PR::GetIpTotalLen(L3_HDR(pkt_msg)))
	{
		LOG(ERROR) << "Invalid IP total length | L2: " 
			<< pkt_msg->l2_pkt_info.l2_data_len
			<< " L3: " << PR::GetIpTotalLen(L3_HDR(pkt_msg));
		return false;
	}

	if (enable_checksum // IPͷ����CRCУ��
		&& !PR::CheckIpHeaderCrc(L3_HDR(pkt_msg), PR::GetIpHdrLen(L3_HDR(pkt_msg))))
	{	
		LOG(ERROR) << "Fail to check IP header CRC";
		return false;
	}

	// �������ݳ���
	L3_DATA_LEN(pkt_msg) = PR::GetIpDataLen(L3_HDR(pkt_msg));

	// ԴIP��Ŀ��IP
	SRC_IP(pkt_msg) = PR::GetSourceIpH(L3_HDR(pkt_msg));
	DST_IP(pkt_msg) = PR::GetDestIpH(L3_HDR(pkt_msg));

	// ��Ƭ��־
	pkt_msg->l3_pkt_info.mf_flag = PR::GetMfFlag(L3_HDR(pkt_msg));

	// ��Ƭ�ڱ����е�ƫ��
	pkt_msg->l3_pkt_info.frag_offset = PR::GetFragOffset(L3_HDR(pkt_msg));

	// ��Ƭ��������ID
	pkt_msg->l3_pkt_info.pkt_id = PR::GetFragPktId(L3_HDR(pkt_msg));

	// ��λL4������ʼλ��
	pkt_msg->l4_pkt_info.l4_hdr = pkt_msg->l3_pkt_info.l3_hdr + 
		PR::GetIpHdrLen(L3_HDR(pkt_msg));

	// ��ȡL4Э������
	pkt_msg->l3_pkt_info.l4_prot = PR::GetIpHdrProt(L3_HDR(pkt_msg));

	// IPЭ��汾
	pkt_msg->l3_pkt_info.ip_ver = PR::GetIpVersion(L3_HDR(pkt_msg));

	// ���ķ���
	pkt_msg->l3_pkt_info.direction = 
		(SRC_IP(pkt_msg) < DST_IP(pkt_msg)) ? PKT_DIR_S2B : PKT_DIR_B2S;

	return true;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����TCP����Ϣ
 * ��  ��: [in][out] pkt_msg ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��22��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ParsePktTcpInfo(PktMsg* pkt_msg)
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

/*------------------------------------------------------------------------------
 * ��  ��: ����UDP����Ϣ
 * ��  ��: [in][out] pkt_msg ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��24��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ParsePktUdpInfo(PktMsg* pkt_msg)
{
	pkt_msg->l4_pkt_info.src_port = PR::GetUdpSrcPortH(pkt_msg->l4_pkt_info.l4_hdr);
	pkt_msg->l4_pkt_info.dst_port = PR::GetUdpDstPortH(pkt_msg->l4_pkt_info.l4_hdr);

	pkt_msg->l4_pkt_info.conn_id = ConnId(IPPROTO_UDP,
		pkt_msg->l3_pkt_info.src_ip,
		pkt_msg->l4_pkt_info.src_port,
		pkt_msg->l3_pkt_info.dst_ip,
		pkt_msg->l4_pkt_info.dst_port);

	L4_DATA_LEN(pkt_msg) = L3_DATA_LEN(pkt_msg) - UDP_HDR_LEN;

	L7_DATA(pkt_msg) = L4_HDR(pkt_msg) + UDP_HDR_LEN;

	return true;
}

}