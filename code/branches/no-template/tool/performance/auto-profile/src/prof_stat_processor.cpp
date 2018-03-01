#include "prof_typedef.hpp"
#include "prof_stat_processor.hpp"
#include "prof_typedef.hpp"

#include <boost/thread.hpp>

namespace BroadInter
{

extern ProfStat g_prof_stat;
extern CmdLinePara g_cmdline_para;

ProcInfo L3ProfStatProcessor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	if (g_cmdline_para.pressure_test)
	{
		g_prof_stat.l3_processed_pkt_num++;
		g_prof_stat.total_l3_processed_pkt_num++;
	}
	else if (g_cmdline_para.prof_measure)
	{
		g_prof_stat.l3_start_proc_time = MicroTimeNow();
	}
	
	return PI_CHAIN_CONTINUE;
}

ProcInfo L4ProfStatProcessor::DoProcess(MsgType msg_type, VoidSP msg_data)
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

	return PI_CHAIN_CONTINUE;
}

ProcInfo L7ProfStatProcessor::DoProcess(MsgType msg_type, VoidSP msg_data)
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

	return PI_CHAIN_CONTINUE;
}

}