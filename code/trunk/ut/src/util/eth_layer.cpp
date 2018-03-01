/*#############################################################################
 * 文件名   : eth_layer.cpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年2月22日
 * 文件描述 : EthLayer类实现
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "eth_layer.hpp"
#include "tmas_typedef.hpp"

namespace BroadInter
{

#define ETH_ADDR_LEN 6

#define pragma pack(1)
struct EthAddr {
	uint8_t		data[ETH_ADDR_LEN];
} eth_addr_t;

struct EthHdr {
	EthAddr	eth_dst;	/* destination address */
	EthAddr	eth_src;	/* source address */
	uint16	eth_type;	/* payload type */
};
#pragma pack()

#define EthPackHdr(h, dst, src, type) do {			\
	EthHdr* eth_pack_p = (EthHdr*)(h);	\
	memmove(&eth_pack_p->eth_dst, dst, ETH_ADDR_LEN);	\
	memmove(&eth_pack_p->eth_src, src, ETH_ADDR_LEN);	\
	eth_pack_p->eth_type = htons(type);			\
} while (0)

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数:
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
EthLayer<ETHERNET_II>::EthLayer() : upper_layer_data_(nullptr)
	, upper_layer_data_len_(0)
{
	char dst_mac[6] = {(char)0xf8, (char)0x0f, (char)0x41, (char)0x08, (char)0x4e, (char)0x24};
	char src_mac[6] = {(char)0x90, (char)0x94, (char)0xe4, (char)0xa7, (char)0x5c, (char)0x86};
	uint16 eth_type = 0x0800;	
	
	EthPackHdr(header_, dst_mac, src_mac, eth_type);
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置上层协议类型
 * 参  数: [in] protocol 协议类型
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
void EthLayer<ETHERNET_II>::SetUpperLayerProtocol(int protocol)
{
	uint16 proto = static_cast<uint16>(protocol);
	uint16* type = reinterpret_cast<uint16*>(header_ + 12);
	*type = proto;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置上层数据
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年02月21日
 *   作者 tom_liu
 *   描述 创建
 ----------------------------------------------------------------------------*/
void EthLayer<ETHERNET_II>::SetUpperLayerData(const char* data, uint32 len)
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
void EthLayer<ETHERNET_II>::GetPacket(void** data, size_t* len)
{
	uint32 pkt_buf_len = ETH_HEADER_LEN + upper_layer_data_len_;
	if (upper_layer_data_len_ < 46)
	{
		pkt_buf_len = 46 + ETH_HEADER_LEN;
	}
	else if (upper_layer_data_len_ > 1500)
	{
		pkt_buf_len = 1500 + ETH_HEADER_LEN;
	}

	char* pkt_buf = new char[pkt_buf_len];

	std::memcpy(pkt_buf, header_, ETH_HEADER_LEN);

	std::memcpy(pkt_buf + ETH_HEADER_LEN, 
		        upper_layer_data_, 
				pkt_buf_len - ETH_HEADER_LEN);
	
	*data = static_cast<void*>(pkt_buf);
	*len = pkt_buf_len;

	// 申请的内存空间由调用者负责释放
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
void EthLayer<ETHERNET_II>::SetDstMacAddr(std::string src_mac)
{
	EthHdr* hdr = (EthHdr*)header_;
	std::memmove(&hdr->eth_dst, src_mac.c_str(), 6);
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
void EthLayer<ETHERNET_II>::SetSrcMacAddr(std::string src_mac)
{
	EthHdr* hdr = (EthHdr*)header_;
	std::memmove(&hdr->eth_src, (void*)src_mac.c_str(), 6);
}

}  // namespace BroadInter