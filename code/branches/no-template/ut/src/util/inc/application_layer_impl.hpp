#ifndef BROADINTER_APPLICATION_LAYER_IMPL
#define BROADINTER_APPLICATION_LAYER_IMPL

namespace BroadInter
{

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::ApplicationLayer()
{
    SetMethod(GET);
    SetProtocol(1, 1);
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::GetPacket(void** data, size_t* length)
{
	// 构造HTTP层数据
    std::ostringstream oss;
    oss << ToString(method_) << ' ' << request_ << " HTTP/" << majar_protocol_ << '.' << secondary_protocol_ << "\r\n";
    for (auto i = headers_.begin(); i != headers_.end(); ++i)
    {
        oss << i->first << ": " << i->second << "\r\n";
    }

    oss << "\r\n";

    std::string http = oss.str() + data_;

	// 构造底层数据
	transport_layer_.SetUpperLayerData(http.c_str(), http.size());
    transport_layer_.GetPacket(data, length);
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::SetMethod(Method method)
{
    method_ = method;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::SetRequest(const std::string& request)
{
    request_ = request;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::SetProtocol(uint8 majar_protocol, uint8 secondary_protocol)
{
    majar_protocol_ = majar_protocol;
    secondary_protocol = secondary_protocol_;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::SetHeader(const std::string& key, const std::string& value)
{
    headers_.insert(std::make_pair(key, value));
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
bool ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::RemoveHeader(const std::string& key)
{
    return headers_.erase(key) > 0;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::SetData(const std::string& data)
{
    data_ = data;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
const char* ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::ToString(Method method)
{
    static const char* const kTable[END] = {"GET", "HEAD", "POST", "PUT", "TRACE", "OPTIONS", "DELETE"};
    return kTable[method];
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
EthLayer<eth_protocol>* ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::GetEthLayer()
{
    return transport_layer_.GetEthLayer();
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
IpLayer<eth_protocol, ip_protocol>* 
ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::GetIpLayer()
{
    return transport_layer_.GetIpLayer();
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
TransportLayer<eth_protocol, ip_protocol, transport_protocol>* 
ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, HTTP>::GetTransportLayer()
{
    return &transport_layer_;
}

//==============================================================================

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::ApplicationLayer()
	: dummy_data_len_(0)
{
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::ApplicationLayer(uint16 dummy_data_len)
	: dummy_data_len_(dummy_data_len)
{
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
EthLayer<eth_protocol>* ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::GetEthLayer()
{
	return transport_layer_.GetEthLayer();
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
IpLayer<eth_protocol, ip_protocol>* 
	ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::GetIpLayer()
{
	return transport_layer_.GetIpLayer();
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
TransportLayer<eth_protocol, ip_protocol, transport_protocol>* 
	ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::GetTransportLayer()
{
	return &transport_layer_;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::SetDummyDataLen(uint16 dummy_data_len)
{
	dummy_data_len_ = dummy_data_len;
}

template<EthProtocol eth_protocol, IpProtocol ip_protocol, TransportProtocol transport_protocol>
void ApplicationLayer<eth_protocol, ip_protocol, transport_protocol, DUMMY>::GetPacket(void** data, size_t* len)
{
	// 构造dummy层数据
	char* dummy_buf = new char[dummy_data_len_];
	
	// 构造底层数据
	transport_layer_.SetUpperLayerData(dummy_buf, dummy_data_len_);
	transport_layer_.GetPacket(data, len);

	delete[] dummy_buf;
}

}  // namespace BroadInter

#endif  // BROADINTER_APPLICATION_LAYER_IMPL
