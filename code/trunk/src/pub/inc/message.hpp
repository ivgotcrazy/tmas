/*#############################################################################
 * 文件名   : message.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月21日
 * 文件描述 : EthMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_MESSAGE
#define BROADINTER_MESSAGE

#include "tmas_typedef.hpp"
#include "connection.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: 报文原始信息
 * 作者：teck_zhou
 * 时间：2014/01/21
 *****************************************************************************/
struct PktEntry
{
	PktEntry() : len(0), buf(0) {}

	// 不要在析构函数中释放buf

	uint32 len;
	char* buf;
};

/******************************************************************************
 * 描述: 报文二层信息
 * 作者：teck_zhou
 * 时间：2014/01/21
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
 * 描述: 报文三层信息
 * 作者：teck_zhou
 * 时间：2014/01/21
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

	//--- IP分片信息
	uint8  mf_flag;
	uint16 frag_offset;
	uint16 pkt_id;
};

/******************************************************************************
 * 描述: 报文四层信息
 * 作者：teck_zhou
 * 时间：2014/01/21
 *****************************************************************************/
struct L4PktInfo
{
	char* l4_hdr;
	uint16 l4_data_len;

	uint16 src_port;
	uint16 dst_port;

	ConnId conn_id;

	//--- TCP信息
	uint32 seq_num;
	uint32 ack_num;

	bool ack_flag;
	bool syn_flag;
	bool rst_flag;
	bool fin_flag;

	//--- UDP信息
};

template<typename Stream>
inline Stream& operator<<(Stream& stream, const L4PktInfo& l4_info)
{
	stream << "[" << l4_info.seq_num << ":" << l4_info.ack_num
		   << ":" << l4_info.l4_data_len << "]";

	return stream;
}

/******************************************************************************
 * 描述: 报文七层信息
 * 作者：teck_zhou
 * 时间：2014/01/21
 *****************************************************************************/
struct L7PktInfo
{
	char* l7_data;
};

/******************************************************************************
 * 描述: 报文信息
 * 作者：teck_zhou
 * 时间：2014/01/21
 *****************************************************************************/
struct PktInfo
{
	uint64 arrive;	// 报文到达时间
	PktEntry pkt;	// 报文原始数据

	L2PktInfo l2_pkt_info;	// 二层信息
	L3PktInfo l3_pkt_info;	// 三层信息
	L4PktInfo l4_pkt_info;	// 四层信息
	L7PktInfo l7_pkt_info;	// 七层信息
};

/******************************************************************************
 * 描述: 报文消息
 * 作者：teck_zhou
 * 时间：2014/01/21
 *****************************************************************************/
typedef PktInfo PktMsg;
typedef boost::shared_ptr<PktMsg> PktMsgSP;

/******************************************************************************
 * 描述: 消息处理器处理的消息类型
 * 作者：teck_zhou
 * 时间：2014/01/12
 *****************************************************************************/
enum MsgType
{
	MSG_PKT = 0,			// 报文消息
	MSG_REINITIALIZE,		// 原始报文消息
	MSG_TCP_CONN_CLOSED		// TCP连接关闭消息
};

/******************************************************************************
 * 描述: 报文方向
 * 作者：teck_zhou
 * 时间：2014/01/12
 *****************************************************************************/
enum PktDirection
{
	PKT_DIR_S2B = 0,	// smaller ip to bigger ip
	PKT_DIR_B2S = 1,	// bigger ip to smaller ip
	
	PKT_DIR_BUTT
};

//================================= 宏定义 ====================================

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
