#ifndef BROADINTER_APPLICATION_LAYER
#define BROADINTER_APPLICATION_LAYER

#include <map>
#include <string>
#include <sstream>

#include "transport_layer.hpp"

namespace BroadInter
{

enum ApplicationProtocol
{
	DUMMY,
	HTTP,
};

template<EthProtocol eth_protocol,
         IpProtocol ip_protocol,
         TransportProtocol transport_protocol,
         ApplicationProtocol application_protocol>
class ApplicationLayer;

template<EthProtocol eth_protocol,
         IpProtocol ip_protocol,
         TransportProtocol transport_protocol>
class ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>
{
public:
	enum Method
	{
		GET = 0,
		HEAD,
		POST,
		PUT,
		TRACE,
		OPTIONS,
		DELETE,
		END
	};

	ApplicationLayer();

	void SetMethod(Method method);
	void SetRequest(const std::string& request);
	void SetProtocol(uint8 majar_protocol, uint8 secondary_protocol);
	void SetHeader(const std::string& key, const std::string& value);
	bool RemoveHeader(const std::string& key);
	void SetData(const std::string& data);

	EthLayer<eth_protocol>* GetEthLayer();
	IpLayer<eth_protocol, ip_protocol>* GetIpLayer();
	TransportLayer<eth_protocol, ip_protocol, transport_protocol>* GetTransportLayer();

	void GetPacket(void** data, size_t* length);

private:
	static const char* ToString(Method method);

private:
	Method method_;
	std::string request_;
	uint8 majar_protocol_;
	uint8 secondary_protocol_;
	std::map<std::string, std::string> headers_;
	std::string data_;

    TransportLayer<eth_protocol, ip_protocol, transport_protocol> transport_layer_;
};

//==============================================================================

template<EthProtocol eth_protocol,
	     IpProtocol ip_protocol,
	     TransportProtocol transport_protocol>
class ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>
{
public:
	ApplicationLayer();
	ApplicationLayer(uint16 dummy_data_len);

	void SetDummyDataLen(uint16 dummy_data_len);

	EthLayer<eth_protocol>* GetEthLayer();
	IpLayer<eth_protocol, ip_protocol>* GetIpLayer();
	TransportLayer<eth_protocol, ip_protocol, transport_protocol>* GetTransportLayer();

	void GetPacket(void** data, size_t* len);

private:
	uint16 dummy_data_len_;

	TransportLayer<eth_protocol, ip_protocol, transport_protocol> transport_layer_;
};

}  // namespace BroadInter

#include "application_layer_impl.hpp"

#endif  // BROADINTER_APPLICATION_LAYER
