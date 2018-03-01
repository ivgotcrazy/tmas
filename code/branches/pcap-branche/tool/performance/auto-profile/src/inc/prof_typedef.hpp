#ifndef BROADINTER_PROF_TYPEDEF
#define BROADINTER_PROF_TYPEDEF

#include <atomic>
#include "tmas_typedef.hpp"
#include <boost/shared_ptr.hpp>

namespace BroadInter
{

struct ProfStat
{
	ptime start_time;
	ptime stop_time;
	ptime l3_start_proc_time;
	ptime l4_start_proc_time;
	ptime l7_start_proc_time;

	std::atomic<uint64> captured_pkt_num;
	std::atomic<uint64> l3_processed_pkt_num;
	std::atomic<uint64> l4_processed_pkt_num;
	std::atomic<uint64> l7_processed_pkt_num;

	std::atomic<uint64> total_captured_pkt_num;
	std::atomic<uint64> total_l3_processed_pkt_num;
	std::atomic<uint64> total_l4_processed_pkt_num;
	std::atomic<uint64> total_l7_processed_pkt_num;

	uint32 time_use_of_layer[11][4];
};

struct CmdLinePara
{
	bool pressure_test;
	bool prof_measure;

	uint32 send_count;
	uint32 micro_sleep_time;
	uint32 nano_sleep_time;
	uint32 concurrent_conns;
};

//==============================================================================
//                                 处理链构建
//==============================================================================

class None;

template<class Next, class Succ>
class HttpMonitor;

typedef HttpMonitor<None, None> HttpMonitorTestType;
typedef boost::shared_ptr<HttpMonitorTestType> HttpMonitorTestTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class UdpMonitor;

typedef UdpMonitor<None, None> UdpMonitorTestType;
typedef boost::shared_ptr<UdpMonitorTestType> UdpMonitorTestTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class TcpMonitor;

typedef TcpMonitor<UdpMonitorType, HttpMonitorTestType> TcpMonitorTestType;
typedef boost::shared_ptr<TcpMonitorTestType> TcpMonitorTestTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class L4ProfStatProcessor;

typedef L4ProfStatProcessor<None, TcpMonitorTestType> L4ProfStatProcessorType;
typedef boost::shared_ptr<L4ProfStatProcessorType> L4ProfStatProcessorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class Ipv4Monitor;

typedef Ipv4Monitor<None, L4ProfStatProcessorType> Ipv4MonitorTestType;
typedef boost::shared_ptr<Ipv4MonitorTestType> Ipv4MonitorTestTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class L3ProfStatProcessor;

typedef L3ProfStatProcessor<None, Ipv4MonitorTestType> L3ProfStatProcessorType;
typedef boost::shared_ptr<L3ProfStatProcessorType> L3ProfStatProcessorTypeSP;

//------------------------------------------------------------------------------

template<class Next, class Succ>
class EthMonitor;

typedef EthMonitor<None, L3ProfStatProcessorType> EthMonitorTestType;
typedef boost::shared_ptr<EthMonitorTestType> EthMonitorTestTypeSP;

//------------------------------------------------------------------------------

template<class PktProcessorType>
class PktDispatcher;

typedef PktDispatcher<EthMonitorTestType> PktDispatcherTestType;
typedef boost::shared_ptr<PktDispatcherTestType> PktDispatcherTestTypeSP;

//------------------------------------------------------------------------------

}

#endif