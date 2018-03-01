/*#############################################################################
 * �ļ���   : eth_layer.cpp
 * ������   : tom_liu
 * ����ʱ�� : 2014��2��22��
 * �ļ����� : EthLayer��ʵ��
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
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
 * ��  ��: ���캯��
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
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
 * ��  ��: �����ϲ�Э������
 * ��  ��: [in] protocol Э������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
void EthLayer<ETHERNET_II>::SetUpperLayerProtocol(int protocol)
{
	uint16 proto = static_cast<uint16>(protocol);
	uint16* type = reinterpret_cast<uint16*>(header_ + 12);
	*type = proto;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �����ϲ�����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��02��21��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
void EthLayer<ETHERNET_II>::SetUpperLayerData(const char* data, uint32 len)
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

	// ������ڴ�ռ��ɵ����߸����ͷ�
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
void EthLayer<ETHERNET_II>::SetDstMacAddr(std::string src_mac)
{
	EthHdr* hdr = (EthHdr*)header_;
	std::memmove(&hdr->eth_dst, src_mac.c_str(), 6);
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
void EthLayer<ETHERNET_II>::SetSrcMacAddr(std::string src_mac)
{
	EthHdr* hdr = (EthHdr*)header_;
	std::memmove(&hdr->eth_src, (void*)src_mac.c_str(), 6);
}

}  // namespace BroadInter