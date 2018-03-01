#include "prof_typedef.hpp"
#include "prof_stat_processor.hpp"
#include "prof_typedef.hpp"

#include <boost/thread.hpp>

namespace BroadInter
{

extern ProfStat g_prof_stat;
extern CmdLinePara g_cmdline_para;

ProcInfo L3ProfStatProcessor::DoProcessPkt(PktMsgSP& pkt_msg)
{
	if (g_cmdline_para.pressure_test)
	{
		g_prof_stat.l3_processed_pkt_num++;
		g_prof_stat.total_l3_processed_pkt_num++;
		return PI_NOT_PROCESSED;
	}
	else if (g_cmdline_para.prof_measure)
	{
		g_prof_stat.l3_start_proc_time = MicroTimeNow();
	}
	
	PassPktToSuccProcessor(pkt_msg);

	return PI_HAS_PROCESSED;
}

ProcInfo L4ProfStatProcessor::DoProcessPkt(PktMsgSP& pkt_msg)
{
	if (g_cmdline_para.pressure_test)
	{
		g_prof_stat.l4_processed_pkt_num++;
		g_prof_stat.total_l4_processed_pkt_num++;
	}
	else if (g_cmdline_para.prof_measure)
	{
		g_prof_stat.l4_start_proc_time = MicroTimeNow();
	}

	PassPktToSuccProcessor(pkt_msg);

	return PI_HAS_PROCESSED;
}

ProcInfo L7ProfStatProcessor::DoProcessPkt(PktMsgSP& pkt_msg)
{
	if (g_cmdline_para.pressure_test)
	{
		g_prof_stat.l7_processed_pkt_num++;
		g_prof_stat.total_l7_processed_pkt_num++;
	}
	else if (g_cmdline_para.prof_measure)
	{
		g_prof_stat.l7_start_proc_time = MicroTimeNow();
	}

	PassPktToSuccProcessor(pkt_msg);

	return PI_HAS_PROCESSED;
}

}