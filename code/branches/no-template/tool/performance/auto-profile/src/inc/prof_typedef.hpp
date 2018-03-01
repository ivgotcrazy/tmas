#ifndef BROADINTER_PROF_TYPEDEF
#define BROADINTER_PROF_TYPEDEF

#include <atomic>
#include "tmas_typedef.hpp"

namespace BroadInter
{

class L3ProfStatProcessor;
class L4ProfStatProcessor;
class L7ProfStatProcessor;

typedef boost::shared_ptr<L3ProfStatProcessor> L3ProfStatProcessorSP;
typedef boost::shared_ptr<L4ProfStatProcessor> L4ProfStatProcessorSP;
typedef boost::shared_ptr<L7ProfStatProcessor> L7ProfStatProcessorSP;

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

}

#endif