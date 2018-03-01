/*#############################################################################
 * 文件名   : transport_layer.hpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年02月21日
 * 文件描述 : TransportLayer声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TRANSPORT_LAYER
#define BROADINTER_TRANSPORT_LAYER

#include "eth_layer.hpp"
#include "ip_layer.hpp"

namespace BroadInter
{

#define TCP_HEADER_LEN 20
#define UDP_HEADER_LEN 8

enum TransportProtocol
{
	TCP,
	UDP,
	SCTP
};

template<EthProtocol eth_protocol,
	IpProtocol ip_protocol,
	TransportProtocol transport_protocol>
class TransportLayer;

template<EthProtocol eth_protocol, IpProtocol ip_protocol>
class TransportLayer<eth_protocol, ip_protocol, TCP>
{
public:
	TransportLayer();
	
	void SetUpperLayerData(const char* data, uint32 len);
	void GetPacket(void** data, size_t* len);
	
	EthLayer<eth_protocol>* GetEthLayer();
	IpLayer<eth_protocol, ip_protocol>* GetIpLayer();

	void SetDstPort(uint16 port);
	void SetSrcPort(uint16 port);
	void SetSeqNum(uint32 num);
	void SetAckNum(uint32 num);

	void SetSynFlag(bool flag);
	void SetAckFlag(bool flag);
	void SetFinFlag(bool flag);
	void SetRstFlag(bool flag);

private:
	void RefreshTcpCheckSum();
	
private:
	IpLayer<eth_protocol, ip_protocol> ip_layer_;

	char* upper_layer_data_;
	uint32 upper_layer_data_len_;

	char header_[TCP_HEADER_LEN];
};

template<EthProtocol eth_protocol, IpProtocol ip_protocol>
class TransportLayer<eth_protocol, ip_protocol, UDP>
{
	void SetUpperLayerProtocol(int protocol);
	void SetData(const void* data, size_t length);
	void GetPacket(void** data, size_t* length);

	EthLayer<eth_protocol>* GetEthLayer();
	IpLayer<eth_protocol, ip_protocol>* GetIpLayer();

private:
	IpLayer<eth_protocol, ip_protocol> ip_layer_;
	char header_[UDP_HEADER_LEN];
};

}  // namespace BroadInter

#include "transport_layer_inl.hpp"

#endif  // BROADINTER_TRANSPORT_LAYER

