/*#############################################################################
 * 文件名   : eth_layer.hpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年02月21日
 * 文件描述 : EthLayer声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描述：数据链路层描述
 * 作者：tom_liu
 * 时间：2014年2月21日
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

	char header_[ETH_HEADER_LEN];  // 本层的头部
};

}  // namespace BroadInter

#endif  // BROADINTER_ETHERNET_LAYER