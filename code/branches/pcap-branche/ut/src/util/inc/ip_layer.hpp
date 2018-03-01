/*#############################################################################
 * 文件名   : ip_layer.hpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年02月21日
 * 文件描述 : IpLayer声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#ifndef BROADINTER_IP_LAYER
#define BROADINTER_IP_LAYER

#include "eth_layer.hpp"
#include "tmas_typedef.hpp"

namespace BroadInter
{

#define IP_HEADER_LEN 20

enum IpProtocol
{
	IPV4,
	IPV6,
};

template<EthProtocol ethernet_protocol, IpProtocol ip_protocol>
class IpLayer;


template<EthProtocol ethernet_protocol>
class IpLayer<ethernet_protocol, IPV4>
{
public:
	IpLayer();
	
	void SetUpperLayerProtocol(int protocol);

	void SetUpperLayerData(const char* data, uint32 len);

	void GetPacket(void** data, size_t* len);

	EthLayer<ethernet_protocol>* GetEthLayer();

	void SetIpVersion(uint8 ver);
	void SetDstIpAddr(uint32 dst_ip);
	void SetSrcIpAddr(uint32 src_ip);
	void SetIpMfFlag(bool Segement);
	void SetIpOffSet(uint16 offset);
	void SetIpBody(char* buf, uint16 len);
	void SetIpId(uint16 id);

	uint32 GetDstIpAddr();
	uint32 GetSrcIpAddr();

private:
	void RefreshIpCheckSum(char* ip_hdr);

private:
	EthLayer<ethernet_protocol> ethernet_layer_;

	char* upper_layer_data_;
	uint32 upper_layer_data_len_;

	char header_[20];
};

}  // BroadInter

#include "ip_layer_inl.hpp"

#endif  // BROADINTER_INTERNET_LAYER

