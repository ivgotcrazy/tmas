/*#############################################################################
 * 文件名   : ip_layer.cpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年2月22日
 * 文件描述 : IpLayer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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

//伪头部
struct PseudoHeader
{
    uint32 source_ip = 0;  // 源Ip地址
    uint32 dest_ip = 0;  // 目的Ip地址
    uint8 user = 0;  
    uint8 protocol = 0;  // 协议
    uint16 length = 0;  // IP data长度
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
 * 描  述: 计算校验和
 * 参  数: [in] sum 数据和
 * 返回值: 校验和
 * 修  改:
 *   时间 2013.11.23
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
static uint16 GetChecksum(uint32 sum)
{
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return static_cast<uint16>(~sum);
}

/*------------------------------------------------------------------------------
 * 描  述: 计算数据的和
 * 参  数: [in] data 数据指针
 *         [in] length 数据长度
 * 返回值: 数据的和
 * 修  改:
 *   时间 2013.11.23
 *   作者 rosan
 *   描述 创建
 *        算法如下：
 *        将传入的数据拆成若干个2个字节的分组，
 *        如果传入的数据长度为奇数，则最后一字节单独为一组
 *        将拆分的各个分组的数字相加
 -----------------------------------------------------------------------------*/ 
static uint32 GetSum(const void* data, uint32 length)
{
    uint32 checksum = 0;  // 和

    const uint16* p = reinterpret_cast<const uint16*>(data);
    while (length > 1)  // 还可以分组
    {
        checksum += *(p++);
        length -= sizeof(uint16);
    }

    if (length != 0)  // 传入的数据长度为奇数
    {
        checksum += *reinterpret_cast<const uint8*>(p);
    }
    
    return checksum;
}

/*------------------------------------------------------------------------------
 * 描  述: 计算IP头部校验和
 * 参  数: [in] data 待校验的数据
 *         [in] length 待校验的数据长度
 * 返回值: IP头部校验和
 * 修  改:
 *   时间 2013.11.23
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
static uint16 GetIpHeaderChecksum(const void* data, uint32 length)
{
    return GetChecksum(GetSum(data, length));
}

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
IpLayer<ethernet_protocol, IPV4>::IpLayer() : upper_layer_data_(nullptr)
	, upper_layer_data_len_(0)
{
	IpPackHdr(header_, 0, 40, 0x737c, 0, 64, 6, inet_addr("192.168.0.191"), inet_addr("192.168.0.223"));
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置上层协议类型
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetUpperLayerProtocol(int protocol)
{
	uint8 proto = static_cast<uint8>(protocol);
	uint8* type = reinterpret_cast<uint8*>(header_ + 10);
	*type = proto;	
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置本层长度
 * 参  数: [in] top_len 上层数据长度 
 * 返回值:
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetUpperLayerData(const char* data, uint32 len)
{
	upper_layer_data_ = const_cast<char*>(data);
	upper_layer_data_len_ = len;

	((IpHdr *)header_)->ip_len = htons(static_cast<uint16>(len) + IP_HEADER_LEN);
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
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::GetPacket(void** data, size_t* len)
{
	//刷新Ip头部检验和
	RefreshIpCheckSum(header_);
	
	// 构造IP层数据
	uint32 ip_buf_len = upper_layer_data_len_ + IP_HEADER_LEN;
	char* ip_buf = new char[ip_buf_len];
	std::memcpy(ip_buf, header_, IP_HEADER_LEN);
	std::memcpy(ip_buf + IP_HEADER_LEN, upper_layer_data_, upper_layer_data_len_);

	// 构造底层数据
	ethernet_layer_.SetUpperLayerData(ip_buf, ip_buf_len);
	ethernet_layer_.GetPacket(data, len);

	delete[] ip_buf;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置Ip协议版本
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpVersion(uint8 num)
{
	((IpHdr *)header_)->ip_v = num;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置目的Mac地址
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetDstIpAddr(uint32 dst_ip)
{
	((IpHdr *)header_)->ip_dst = htonl(dst_ip);
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置源Mac地址
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetSrcIpAddr(uint32 src_ip)
{
	((IpHdr *)header_)->ip_src= htonl(src_ip);
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置Ip报文分片标志
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpId(uint16 id)
{
	((IpHdr *)header_)->ip_id = htons(id);
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置Ip报文分片标志
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
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
 * 描  述: 设置Ip报文分片偏移
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::SetIpOffSet(uint16 offset)
{
	IpHdr* ip_hdr = (IpHdr *)header_;
	ip_hdr->ip_off = htons((ntohs(ip_hdr->ip_off) & 0x2000) | (offset & 0x1FFF));
}

/*------------------------------------------------------------------------------
 * 描  述: 刷新IP头部的CRC校验
 * 参  数:
 * 返回值: bool 校验和正确
 *         false 检验和错误
 * 修  改:
 *   时间 2014年01月07日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
void IpLayer<ethernet_protocol, IPV4>::RefreshIpCheckSum(char* ip_hdr)
{
	IpHdr* hdr = (IpHdr *)ip_hdr;
	hdr->ip_sum = 0; //首先重置checksum为0
	hdr->ip_sum = GetIpHeaderChecksum(hdr, (hdr->ip_hl) * 4);
}

/*------------------------------------------------------------------------------
 * 描  述: 目标Ip地址访问器
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年01月07日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
uint32 IpLayer<ethernet_protocol, IPV4>::GetDstIpAddr()
{
	return ntohl(((IpHdr *)header_)->ip_dst);
}

/*------------------------------------------------------------------------------
 * 描  述: 源Ip地址访问器
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年01月07日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
uint32 IpLayer<ethernet_protocol, IPV4>::GetSrcIpAddr()
{
	return ntohl(((IpHdr *)header_)->ip_dst);
}


/*------------------------------------------------------------------------------
 * 描  述: EthLayer访问器接口
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年01月07日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
template <EthProtocol ethernet_protocol>
EthLayer<ethernet_protocol>* IpLayer<ethernet_protocol, IPV4>::GetEthLayer()
{
	return &ethernet_layer_;
}

}

#endif