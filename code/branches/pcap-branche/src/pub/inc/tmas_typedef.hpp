/*#############################################################################
 * 文件名   : tmas_typedef.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2013年12月23日
 * 文件描述 : 类型定义
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TMAS_TYPEDEF
#define BROADINTER_TMAS_TYPEDEF

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace BroadInter
{

#define TMAS_ETH_TYPE_IP		0x0800
#define TMAS_ETH_TYPE_8021Q		0x8100
#define TMAS_IP_VER_4			0x4

//==============================================================================
//                               boost库类型重定义
//==============================================================================
namespace fs = boost::filesystem;

typedef boost::int8_t   int8;
typedef boost::int16_t  int16;
typedef boost::int32_t  int32;
typedef boost::int64_t  int64;
typedef boost::uint8_t  uint8;
typedef boost::uint16_t uint16;
typedef boost::uint32_t uint32;
typedef boost::uint64_t uint64;
typedef boost::int64_t  size_type;

typedef boost::asio::ip::address_v4		 ipv4_address;
typedef boost::asio::ip::address		 ip_address;
typedef boost::posix_time::ptime		 ptime;
typedef boost::posix_time::time_duration time_duration;
typedef boost::system::error_code		 error_code;
typedef boost::asio::io_service			 io_service;
typedef boost::asio::ip::tcp::socket	 tcp_socket;
typedef boost::asio::ip::udp::socket	 udp_socket;
typedef boost::asio::ip::tcp::endpoint	 tcp_endpoint;
typedef boost::asio::ip::udp::endpoint	 udp_endpoint;
typedef boost::asio::deadline_timer		 deadline_timer;

typedef boost::shared_ptr<boost::thread> ThreadSP;

//==============================================================================
//                                 内部类型重定义
//==============================================================================

class BasicConfigParser;
class HttpRecombinder;
class HttpRunSession;
class Ipv4FragReassembler;
class PktCapturer;
class PktEntry;
class PktInfo;

typedef boost::shared_ptr<BasicConfigParser>	BasicConfigParserSP;
typedef boost::shared_ptr<HttpRecombinder>		HttpRecombinderSP;
typedef boost::shared_ptr<HttpRunSession>		HttpRunSessionSP;
typedef boost::shared_ptr<Ipv4FragReassembler>	Ipv4FragReassemblerSP;
typedef boost::shared_ptr<PktCapturer>			PktCapturerSP;
typedef boost::shared_ptr<PktEntry>				PktEntrySP;
typedef boost::shared_ptr<PktInfo>				PktInfoSP;

//==============================================================================
//                                 处理链构建
//==============================================================================

class None;

template<class Next, class Succ>
class HttpMonitor;

typedef HttpMonitor<None, None> HttpMonitorType;
typedef boost::shared_ptr<HttpMonitorType> HttpMonitorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class UdpMonitor;

typedef UdpMonitor<None, None> UdpMonitorType;
typedef boost::shared_ptr<UdpMonitorType> UdpMonitorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class TcpMonitor;

typedef TcpMonitor<UdpMonitorType, HttpMonitorType> TcpMonitorType;
typedef boost::shared_ptr<TcpMonitorType> TcpMonitorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class Ipv4Monitor;

typedef Ipv4Monitor<None, TcpMonitorType> Ipv4MonitorType;
typedef boost::shared_ptr<Ipv4MonitorType> Ipv4MonitorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class EthMonitor;

typedef EthMonitor<None, Ipv4MonitorType> EthMonitorType;
typedef boost::shared_ptr<EthMonitorType> EthMonitorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class TcpConnAnalyzer;

typedef TcpConnAnalyzer<None, HttpMonitorType> TcpConnAnalyzerType;
typedef boost::shared_ptr<TcpConnAnalyzerType> TcpConnAnalyzerTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class TcpConnReorder;

typedef TcpConnReorder<None, TcpConnAnalyzerType> TcpConnReorderType;
typedef boost::shared_ptr<TcpConnReorderType> TcpConnReorderTypeSP;

//------------------------------------------------------------------------------

template<class PktProcessorType>
class PktDispatcher;

typedef PktDispatcher<EthMonitorType> PktDispatcherType;
typedef boost::shared_ptr<PktDispatcherType> PktDispatcherTypeSP;

//------------------------------------------------------------------------------

}

#endif