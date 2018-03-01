/*#############################################################################
 * �ļ���   : transport_layer_inl.hpp
 * ������   : tom_liu
 * ����ʱ�� : 2014��2��22��
 * �ļ����� : TransportLayer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ����IP���ݵ�У���
 * ��  ��: [in] data ��У�������
 *         [in] length ��У������ݳ���
 *         [in] pseudo_header ��У���α�ײ�
 * ����ֵ: IP���ݵ�У���
 * ��  ��:
 *   ʱ�� 2013.11.23
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
static uint16 GetIpDataChecksum(const void* data, uint32 length, 
                                const PseudoHeader& pseudo_header)
{
    return GetChecksum(GetSum(data, length) 
        + GetSum(&pseudo_header, sizeof(PseudoHeader)));
}

//////////////////////////////////Tcp////////////////////////////////////
/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
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
 * ��  ��: ���ñ������ݳ���
 * ��  ��:
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
void TransportLayer<eth_proto, ip_proto, TCP>::SetUpperLayerData(const char* data, uint32 len)																
{
	upper_layer_data_ = const_cast<char*>(data);
	upper_layer_data_len_ = len;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ�������ݻ���
 * ��  ��: [out] data �����ַ
 *         [out] length ���泤��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
void TransportLayer<eth_proto, ip_proto, TCP>::GetPacket(void** data, size_t* len)
{
	RefreshTcpCheckSum();

	// ����TCP������
	uint32 tcp_buf_len = upper_layer_data_len_ + TCP_HEADER_LEN;
	char* tcp_buf = new char[tcp_buf_len];

	std::memcpy(tcp_buf, header_, TCP_HEADER_LEN);

	std::memcpy(tcp_buf + TCP_HEADER_LEN, 
		        upper_layer_data_, 
				upper_layer_data_len_);
	
	// ����ײ�����
	ip_layer_.SetUpperLayerData(tcp_buf, tcp_buf_len);
	ip_layer_.GetPacket(data, len);

	delete[] tcp_buf;
}

/*------------------------------------------------------------------------------
 * ��  ��: EthLayer�������ӿ�
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
EthLayer<eth_proto>* TransportLayer<eth_proto, ip_proto, TCP>::GetEthLayer()
{
	return ip_layer_.GetEthLayer(); 
}

/*------------------------------------------------------------------------------
 * ��  ��: IpLayer�������ӿ�
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto>
IpLayer<eth_proto, ip_proto>* TransportLayer<eth_proto, ip_proto, TCP>::GetIpLayer()
{
	return &ip_layer_;		
}

/*------------------------------------------------------------------------------
 * ��  ��: ����Ŀ��˿�
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetDstPort(uint16 port)
{
	((TcpHdr*)header_)->th_dport = htons(port);
}

/*------------------------------------------------------------------------------
 * ��  ��: ����Դ�˿�
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetSrcPort(uint16 port)
{
	((TcpHdr*)header_)->th_sport = htons(port);
}

/*------------------------------------------------------------------------------
 * ��  ��: ���� ���к�
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetSeqNum(uint32 num)
{
	((TcpHdr*)header_)->th_seq = htonl(num);
}

/*------------------------------------------------------------------------------
 * ��  ��: ���û�Ӧ��
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetAckNum(uint32 num)
{
	((TcpHdr*)header_)->th_ack = htonl(num);
}

/*------------------------------------------------------------------------------
 * ��  ��: ˢ�¼����
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014.02.22
 *   ���� tom_liu
 *   ���� ����
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
	pseudo.length = tcp_buf_len; //tcp���峤��

	//��������
	uint16 checksum = GetIpDataChecksum(tcp_buf, tcp_buf_len, pseudo);

	((TcpHdr*)header_)->th_sum = checksum;

	delete[] tcp_buf;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����SYN��־λ
 * ��  ��: [in] flag ��־ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��25��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetSynFlag(bool flag)
{
	((tcphdr*)header_)->syn = flag;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����ACK��־λ
 * ��  ��: [in] flag ��־ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��25��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetAckFlag(bool flag)
{
	((tcphdr*)header_)->ack = flag;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����FIN��־λ
 * ��  ��: [in] flag ��־ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��25��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetFinFlag(bool flag)
{
	((tcphdr*)header_)->fin = flag;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����RST��־λ
 * ��  ��: [in] flag ��־ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��25��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol eth_proto, IpProtocol ip_proto> 
void TransportLayer<eth_proto, ip_proto, TCP>::SetRstFlag(bool flag)
{
	((tcphdr*)header_)->rst = flag;
}

}

#endif
