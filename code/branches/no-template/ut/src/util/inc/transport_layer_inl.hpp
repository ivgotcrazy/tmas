/*#############################################################################
 * 文件名   : transport_layer_inl.hpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年2月22日
 * 文件描述 : TransportLayer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TRANSPORT_LAYER_INL
#define BROADINTER_TRANSPORT_LAYER_INL

namespace BroadInter	
{

#pragma pack(1)
struct TcpHdr {
	uint16	th_sport;	/* source port */
	uint16	th_dport;	/* destination port */
	uint32	th_seq;		/* sequence number */
	uint32	th_ack;		/* acknowledgment number */
	uint8	th_x2:4,	/* (unused) */
			th_off:4;	/* data offset */
	uint8	th_flags;	/* control flags */
	uint16	th_win;		/* window */
	uint16	th_sum;		/* checksum */
	uint16	th_urp;		/* urgent pointer */
};

struct UdpHdr {
	uint16	uh_sport;	/* source port */
	uint16	uh_dport;	/* destination port */
	uint16	uh_ulen;	/* udp length (including header) */
	uint16	uh_sum;		/* udp checksum */
};
#pragma pack()

#define TcpPackHdr(hdr, sport, dport, seq, ack, flags, win, urp) do {	\
	TcpHdr *tcp_pack_p = (TcpHdr*)(hdr);			\
	tcp_pack_p->th_sport = htons(sport);			\
	tcp_pack_p->th_dport = htons(dport);			\
	tcp_pack_p->th_seq = htonl(seq);				\
	tcp_pack_p->th_ack = htonl(ack);				\
	tcp_pack_p->th_x2 = 0; tcp_pack_p->th_off = 5;	\
	tcp_pack_p->th_flags = flags;					\
	tcp_pack_p->th_win = htons(win);				\
	tcp_pack_p->th_urp = htons(urp);				\
} while (0)

#define UdpPackHdr(hdr, sport, dport, ulen) do {	\
	UdpHdr *udp_pack_p = (UdpHdr*)(hdr);			\
	udp_pack_p->uh_sport = htons(sport);			\
	udp_pack_p->uh_dport = htons(dport);			\
	udp_pack_p->uh_ulen = htons(ulen);				\
} while (0)

/*------------------------------------------------------------------------------
 * 描  述: 计算IP数据的校验和
 * 参  数: [in] data 待校验的数据
 *         [in] length 待校验的数据长度
 *         [in] pseudo_header 待校验的伪首部
 * 返回值: IP数据的校验和
 * 修  改:
 *   时间 2013.11.23
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
static uint16 GetIpDataChecksum(const void* data, uint32 length, 
                                const PseudoHeader& pseudo_header)
{
    return GetChecksum(GetSum(data, length) 
        + GetSum(&pseudo_header, sizeof(PseudoHeader)));
}

//////////////////////////////////Tcp////////////////////////////////////
/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
TransportLayer<eth_proto, ip_proto, TCP>::TransportLayer() 
	: upper_layer_data_(nullptr)
	, upper_layer_data_len_(0)
{
	TcpPackHdr(header_, 8382, 8383, 0x1234, 0x1345, 0, 2048, 0);

	tcphdr* hdr = (tcphdr*)header_;
	hdr->doff = 5;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置本层数据长度
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
void TransportLayer<eth_proto, ip_proto, TCP>::SetUpperLayerData(const char* data, uint32 len)																
{
	upper_layer_data_ = const_cast<char*>(data);
	upper_layer_data_len_ = len;
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取本层数据缓存
 * 参  数: [out] data 缓存地址
 *         [out] length 缓存长度
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
void TransportLayer<eth_proto, ip_proto, TCP>::GetPacket(void** data, size_t* len)
{
	RefreshTcpCheckSum();

	// 构造TCP层数据
	uint32 tcp_buf_len = upper_layer_data_len_ + TCP_HEADER_LEN;
	char* tcp_buf = new char[tcp_buf_len];

	std::memcpy(tcp_buf, header_, TCP_HEADER_LEN);

	std::memcpy(tcp_buf + TCP_HEADER_LEN, 
		        upper_layer_data_, 
				upper_layer_data_len_);
	
	// 构造底层数据
	ip_layer_.SetUpperLayerData(tcp_buf, tcp_buf_len);
	ip_layer_.GetPacket(data, len);

	delete[] tcp_buf;
}

/*------------------------------------------------------------------------------
 * 描  述: EthLayer访问器接口
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
EthLayer<eth_proto>* TransportLayer<eth_proto, ip_proto, TCP>::GetEthLayer()
{
	return ip_layer_.GetEthLayer(); 
}

/*------------------------------------------------------------------------------
 * 描  述: IpLayer访问器接口
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
IpLayer<eth_proto, ip_proto>* TransportLayer<eth_proto, ip_proto, TCP>::GetIpLayer()
{
	return &ip_layer_;		
}

/*------------------------------------------------------------------------------
 * 描  述: 设置目标端口
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetDstPort(uint16 port)
{
	((TcpHdr*)header_)->th_dport = htons(port);
}

/*------------------------------------------------------------------------------
 * 描  述: 设置源端口
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetSrcPort(uint16 port)
{
	((TcpHdr*)header_)->th_sport = htons(port);
}

/*------------------------------------------------------------------------------
 * 描  述: 设置 序列号
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetSeqNum(uint32 num)
{
	((TcpHdr*)header_)->th_seq = htonl(num);
}

/*------------------------------------------------------------------------------
 * 描  述: 设置回应号
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetAckNum(uint32 num)
{
	((TcpHdr*)header_)->th_ack = htonl(num);
}

/*------------------------------------------------------------------------------
 * 描  述: 刷新检验和
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014.02.22
 *   作者 tom_liu
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::RefreshTcpCheckSum()
{
	uint32 tcp_buf_len = upper_layer_data_len_ + TCP_HEADER_LEN;
	char* tcp_buf = new char[tcp_buf_len];
	std::memcpy(tcp_buf, header_, TCP_HEADER_LEN);
	std::memcpy(tcp_buf + TCP_HEADER_LEN, upper_layer_data_, upper_layer_data_len_);

	PseudoHeader pseudo;
	pseudo.source_ip = ip_layer_.GetDstIpAddr();
	pseudo.dest_ip = ip_layer_.GetDstIpAddr();
	pseudo.protocol = 6;
	pseudo.user = 0;
	pseudo.length = tcp_buf_len; //tcp整体长度

	//计算检验和
	uint16 checksum = GetIpDataChecksum(tcp_buf, tcp_buf_len, pseudo);

	((TcpHdr*)header_)->th_sum = checksum;

	delete[] tcp_buf;
}

/*------------------------------------------------------------------------------
 * 描  述: 设置SYN标志位
 * 参  数: [in] flag 标志值
 * 返回值: 
 * 修  改:
 *   时间 2014年02月25日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetSynFlag(bool flag)
{
	((tcphdr*)header_)->syn = flag;
}

/*------------------------------------------------------------------------------
 * 描  述: 设置ACK标志位
 * 参  数: [in] flag 标志值
 * 返回值: 
 * 修  改:
 *   时间 2014年02月25日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetAckFlag(bool flag)
{
	((tcphdr*)header_)->ack = flag;
}

/*------------------------------------------------------------------------------
 * 描  述: 设置FIN标志位
 * 参  数: [in] flag 标志值
 * 返回值: 
 * 修  改:
 *   时间 2014年02月25日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetFinFlag(bool flag)
{
	((tcphdr*)header_)->fin = flag;
}

/*------------------------------------------------------------------------------
 * 描  述: 设置RST标志位
 * 参  数: [in] flag 标志值
 * 返回值: 
 * 修  改:
 *   时间 2014年02月25日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetRstFlag(bool flag)
{
	((tcphdr*)header_)->rst = flag;
}

}

#endif
