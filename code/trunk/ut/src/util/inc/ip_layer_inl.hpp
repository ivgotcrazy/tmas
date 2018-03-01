/*#############################################################################
 * �ļ���   : ip_layer.cpp
 * ������   : tom_liu
 * ����ʱ�� : 2014��2��22��
 * �ļ����� : IpLayer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IP_LAYER_INL
#define BROADINTER_IP_LAYER_INL

namespace BroadInter
{

#pragma pack(1)
struct IpHdr {
	uint8	ip_hl:4,	/* header length (incl any options) */
	        ip_v:4;		/* version */
	uint8	ip_tos;		/* type of service */
	uint16	ip_len;		/* total length (incl header) */
	uint16	ip_id;		/* identification */
	uint16	ip_off;		/* fragment offset and flags */
	uint8	ip_ttl;		/* time to live */
	uint8	ip_p;		/* protocol */
	uint16	ip_sum;		/* checksum */
	uint32	ip_src;		/* source address */
	uint32	ip_dst;		/* destination address */
};

//αͷ��
struct PseudoHeader
{
    uint32 source_ip = 0;  // ԴIp��ַ
    uint32 dest_ip = 0;  // Ŀ��Ip��ַ
    uint8 user = 0;  
    uint8 protocol = 0;  // Э��
    uint16 length = 0;  // IP data����
};
#pragma pack()

#define IpPackHdr(hdr, tos, len, id, off, ttl, p, src, dst) do {	\
	IpHdr* ip_pack_p = (IpHdr*)(hdr);		\
	ip_pack_p->ip_v = 4; ip_pack_p->ip_hl = 5;			\
	ip_pack_p->ip_tos = tos; ip_pack_p->ip_len = htons(len);	\
 	ip_pack_p->ip_id = htons(id); ip_pack_p->ip_off = htons(off);	\
	ip_pack_p->ip_ttl = ttl; ip_pack_p->ip_p = p;			\
	ip_pack_p->ip_src = src; ip_pack_p->ip_dst = dst;		\
} while (0)

//////////////////CheckSum//////////////////////////////////////////////////
/*------------------------------------------------------------------------------
 * ��  ��: ����У���
 * ��  ��: [in] sum ���ݺ�
 * ����ֵ: У���
 * ��  ��:
 *   ʱ�� 2013.11.23
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
static uint16 GetChecksum(uint32 sum)
{
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return static_cast<uint16>(~sum);
}

/*------------------------------------------------------------------------------
 * ��  ��: �������ݵĺ�
 * ��  ��: [in] data ����ָ��
 *         [in] length ���ݳ���
 * ����ֵ: ���ݵĺ�
 * ��  ��:
 *   ʱ�� 2013.11.23
 *   ���� rosan
 *   ���� ����
 *        �㷨���£�
 *        ����������ݲ�����ɸ�2���ֽڵķ��飬
 *        �����������ݳ���Ϊ�����������һ�ֽڵ���Ϊһ��
 *        ����ֵĸ���������������
 -----------------------------------------------------------------------------*/ 
static uint32 GetSum(const void* data, uint32 length)
{
    uint32 checksum = 0;  // ��

    const uint16* p = reinterpret_cast<const uint16*>(data);
    while (length > 1)  // �����Է���
    {
        checksum += *(p++);
        length -= sizeof(uint16);
    }

    if (length != 0)  // ��������ݳ���Ϊ����
    {
        checksum += *reinterpret_cast<const uint8*>(p);
    }
    
    return checksum;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����IPͷ��У���
 * ��  ��: [in] data ��У�������
 *         [in] length ��У������ݳ���
 * ����ֵ: IPͷ��У���
 * ��  ��:
 *   ʱ�� 2013.11.23
 *   ���� rosan
 *   ���� ����
 -----------------------------------------------------------------------------*/ 
static uint16 GetIpHeaderChecksum(const void* data, uint32 length)
{
    return GetChecksum(GetSum(data, length));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
IpLayer<ethernet_protocol, IPV4>::IpLayer() : upper_layer_data_(nullptr)
	, upper_layer_data_len_(0)
{
	IpPackHdr(header_, 0, 40, 0x737c, 0, 64, 6, inet_addr("192.168.0.191"), inet_addr("192.168.0.223"));
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����ϲ�Э������
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetUpperLayerProtocol(int protocol)
{
	uint8 proto = static_cast<uint8>(protocol);
	uint8* type = reinterpret_cast<uint8*>(header_ + 10);
	*type = proto;	
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ñ��㳤��
 * ��  ��: [in] top_len �ϲ����ݳ��� 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetUpperLayerData(const char* data, uint32 len)
{
	upper_layer_data_ = const_cast<char*>(data);
	upper_layer_data_len_ = len;

	((IpHdr *)header_)->ip_len = htons(static_cast<uint16>(len) + IP_HEADER_LEN);
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
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::GetPacket(void** data, size_t* len)
{
	//ˢ��Ipͷ�������
	RefreshIpCheckSum(header_);
	
	// ����IP������
	uint32 ip_buf_len = upper_layer_data_len_ + IP_HEADER_LEN;
	char* ip_buf = new char[ip_buf_len];
	std::memcpy(ip_buf, header_, IP_HEADER_LEN);
	std::memcpy(ip_buf + IP_HEADER_LEN, upper_layer_data_, upper_layer_data_len_);

	// ����ײ�����
	ethernet_layer_.SetUpperLayerData(ip_buf, ip_buf_len);
	ethernet_layer_.GetPacket(data, len);

	delete[] ip_buf;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����IpЭ��汾
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpVersion(uint8 num)
{
	((IpHdr *)header_)->ip_v = num;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����Ŀ��Mac��ַ
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetDstIpAddr(uint32 dst_ip)
{
	((IpHdr *)header_)->ip_dst = htonl(dst_ip);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����ԴMac��ַ
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetSrcIpAddr(uint32 src_ip)
{
	((IpHdr *)header_)->ip_src= htonl(src_ip);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����Ip���ķ�Ƭ��־
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpId(uint16 id)
{
	((IpHdr *)header_)->ip_id = htons(id);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����Ip���ķ�Ƭ��־
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpMfFlag(bool Segement)
{
	IpHdr *ip_hdr = (IpHdr *)header_;
	if (Segement)
	{
		ip_hdr->ip_off =  htons((ntohs(ip_hdr->ip_off) | 0x2000));
	}
	else
	{
		ip_hdr->ip_off =  htons((ntohs(ip_hdr->ip_off) & 0x1FFF));
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����Ip���ķ�Ƭƫ��
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpOffSet(uint16 offset)
{
	IpHdr* ip_hdr = (IpHdr *)header_;
	ip_hdr->ip_off = htons((ntohs(ip_hdr->ip_off) & 0x2000) | (offset & 0x1FFF));
}

/*------------------------------------------------------------------------------
 * ��  ��: ˢ��IPͷ����CRCУ��
 * ��  ��:
 * ����ֵ: bool У�����ȷ
 *         false ����ʹ���
 * ��  ��:
 *   ʱ�� 2014��01��07��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::RefreshIpCheckSum(char* ip_hdr)
{
	IpHdr* hdr = (IpHdr *)ip_hdr;
	hdr->ip_sum = 0; //��������checksumΪ0
	hdr->ip_sum = GetIpHeaderChecksum(hdr, (hdr->ip_hl) * 4);
}

/*------------------------------------------------------------------------------
 * ��  ��: Ŀ��Ip��ַ������
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��07��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
uint32 IpLayer<ethernet_protocol, IPV4>::GetDstIpAddr()
{
	return ntohl(((IpHdr *)header_)->ip_dst);
}

/*------------------------------------------------------------------------------
 * ��  ��: ԴIp��ַ������
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��07��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
uint32 IpLayer<ethernet_protocol, IPV4>::GetSrcIpAddr()
{
	return ntohl(((IpHdr *)header_)->ip_dst);
}


/*------------------------------------------------------------------------------
 * ��  ��: EthLayer�������ӿ�
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��01��07��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
EthLayer<ethernet_protocol>* IpLayer<ethernet_protocol, IPV4>::GetEthLayer()
{
	return &ethernet_layer_;
}

}

#endif