/*#############################################################################
 * �ļ���   : eth_layer.hpp
 * ������   : tom_liu
 * ����ʱ�� : 2014��02��21��
 * �ļ����� : EthLayer����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_ETH_LAYER
#define BROADINTER_ETH_LAYER

#include <cstddef>
#include <string>

#include <boost/integer.hpp>

namespace BroadInter
{

typedef boost::uint32_t uint32;
typedef boost::uint16_t uint16;

enum EthProtocol
{
	ETHERNET_II,
};

#define ETH_HEADER_LEN 14

template<EthProtocol eth_protocol>
class EthLayer;

/******************************************************************************
 * ������������·������
 * ���ߣ�tom_liu
 * ʱ�䣺2014��2��21��
 *****************************************************************************/
template<>
class EthLayer<ETHERNET_II>
{
public:
	EthLayer();
	
	void SetUpperLayerProtocol(int protocol);

	void SetUpperLayerData(const char* data, uint32 len);

	void GetPacket(void** data, size_t* len);

	void SetSrcMacAddr(std::string src_mac);
	
	void SetDstMacAddr(std::string dst_mac);

private:

	char* upper_layer_data_;
	uint32 upper_layer_data_len_;

	char header_[ETH_HEADER_LEN];  // �����ͷ��
};

}  // namespace BroadInter

#endif  // BROADINTER_ETHERNET_LAYER