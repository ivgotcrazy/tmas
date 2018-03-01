/*#############################################################################
 * �ļ���   : message.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��21��
 * �ļ����� : EthMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_MESSAGE
#define BROADINTER_MESSAGE

#include "tmas_typedef.hpp"
#include "connection.hpp"

namespace BroadInter
{

/******************************************************************************
 * ����: ����ԭʼ��Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
struct PktEntry
{
	PktEntry() : len(0), buf(0) {}

	// ��Ҫ�������������ͷ�buf

	uint32 len;
	char* buf;
};

/******************************************************************************
 * ����: ���Ķ�����Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
#define MAC_LEN	6

struct L2PktInfo
{
	char* l2_hdr;
	uint16 l2_data_len;
	uint16 eth_hdr_len;
	
	char src_mac[MAC_LEN];
	char dst_mac[MAC_LEN];
	uint16 l3_prot;
	uint8 vlan_num;
};

/******************************************************************************
 * ����: ����������Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
struct L3PktInfo
{
	char* l3_hdr;
	uint16 l3_data_len;
	uint8 ip_ver;
	uint8 l4_prot;

	uint32 src_ip;
	uint32 dst_ip;

	uint8 direction;

	//--- IP��Ƭ��Ϣ
	uint8  mf_flag;
	uint16 frag_offset;
	uint16 pkt_id;
};

/******************************************************************************
 * ����: �����Ĳ���Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
struct L4PktInfo
{
	char* l4_hdr;
	uint16 l4_data_len;

	uint16 src_port;
	uint16 dst_port;

	ConnId conn_id;

	//--- TCP��Ϣ
	uint32 seq_num;
	uint32 ack_num;

	bool ack_flag;
	bool syn_flag;
	bool rst_flag;
	bool fin_flag;

	//--- UDP��Ϣ
};

template<typename Stream>
inline Stream& operator<<(Stream& stream, const L4PktInfo& l4_info)
{
	stream << "[" << l4_info.seq_num << ":" << l4_info.ack_num
		   << ":" << l4_info.l4_data_len << "]";

	return stream;
}

/******************************************************************************
 * ����: �����߲���Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
struct L7PktInfo
{
	char* l7_data;
};

/******************************************************************************
 * ����: ������Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
struct PktInfo
{
	uint64 arrive;	// ���ĵ���ʱ��
	PktEntry pkt;	// ����ԭʼ����

	L2PktInfo l2_pkt_info;	// ������Ϣ
	L3PktInfo l3_pkt_info;	// ������Ϣ
	L4PktInfo l4_pkt_info;	// �Ĳ���Ϣ
	L7PktInfo l7_pkt_info;	// �߲���Ϣ
};

/******************************************************************************
 * ����: ������Ϣ
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/21
 *****************************************************************************/
typedef PktInfo PktMsg;
typedef boost::shared_ptr<PktMsg> PktMsgSP;

/******************************************************************************
 * ����: ��Ϣ�������������Ϣ����
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/12
 *****************************************************************************/
enum MsgType
{
	MSG_PKT = 0,			// ������Ϣ
	MSG_REINITIALIZE,		// ԭʼ������Ϣ
	MSG_TCP_CONN_CLOSED		// TCP���ӹر���Ϣ
};

/******************************************************************************
 * ����: ���ķ���
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/01/12
 *****************************************************************************/
enum PktDirection
{
	PKT_DIR_S2B = 0,	// smaller ip to bigger ip
	PKT_DIR_B2S = 1,	// bigger ip to smaller ip
	
	PKT_DIR_BUTT
};

//================================= �궨�� ====================================

#define L2_DATA_LEN(pkt_msg)	((pkt_msg)->l2_pkt_info.l2_data_len)
#define ETH_HDR_LEN(pkt_msg)	((pkt_msg)->l2_pkt_info.eth_hdr_len)

#define L3_HDR(pkt_msg)			((pkt_msg)->l3_pkt_info.l3_hdr)
#define L3_DATA_LEN(pkt_msg)	((pkt_msg)->l3_pkt_info.l3_data_len)
#define FRAG_OFFSET(pkt_msg)	((pkt_msg)->l3_pkt_info.frag_offset)
#define SRC_IP(pkt_msg)			((pkt_msg)->l3_pkt_info.src_ip)
#define DST_IP(pkt_msg)			((pkt_msg)->l3_pkt_info.dst_ip)
#define L4_PROT(pkt_msg)		((pkt_msg)->l3_pkt_info.l4_prot)

#define L4_HDR(pkt_msg)			((pkt_msg)->l4_pkt_info.l4_hdr)
#define L4_DATA_LEN(pkt_msg)	((pkt_msg)->l4_pkt_info.l4_data_len)
#define	SEQ_NUM(pkt_msg)		((pkt_msg)->l4_pkt_info.seq_num)
#define ACK_NUM(pkt_msg)		((pkt_msg)->l4_pkt_info.ack_num)
#define ACK_FLAG(pkt_msg)		((pkt_msg)->l4_pkt_info.ack_flag)
#define SYN_FLAG(pkt_msg)		((pkt_msg)->l4_pkt_info.syn_flag)
#define RST_FLAG(pkt_msg)		((pkt_msg)->l4_pkt_info.rst_flag)
#define FIN_FLAG(pkt_msg)		((pkt_msg)->l4_pkt_info.fin_flag)
#define CONN_ID(pkt_msg)		((pkt_msg)->l4_pkt_info.conn_id)

#define L7_DATA(pkt_msg)		((pkt_msg)->l7_pkt_info.l7_data)

}

#endif
