#include <string>
#include <glog/logging.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>  

#include "tmas_config_parser.hpp"
#include "pkt_dispatcher.hpp"
#include "eth_monitor.hpp"
#include "ip_monitor.hpp"
#include "l4_monitor.hpp"
#include "l7_monitor.hpp"
#include "database_recorder.hpp"
#include "tmas_assert.hpp"
#include "pkt_capture_simulator.hpp"
#include "prof_stat_processor.hpp"
#include "prof_typedef.hpp"
#include "tmas_util.hpp"

using namespace BroadInter;

namespace BroadInter
{
	ProfStat g_prof_stat;
	CmdLinePara g_cmdline_para;
}

void GlobalDataInit()
{
	std::memset(&g_cmdline_para, 0x0, sizeof(g_cmdline_para));
	g_cmdline_para.concurrent_conns = 1;

	std::memset(&g_prof_stat, 0x0, sizeof(g_prof_stat));

	g_prof_stat.start_time		   = MicroTimeNow();
	g_prof_stat.stop_time		   = MicroTimeNow();
	g_prof_stat.l3_start_proc_time = MicroTimeNow();
	g_prof_stat.l4_start_proc_time = MicroTimeNow();
	g_prof_stat.l7_start_proc_time = MicroTimeNow();
}

bool ParseCmdLinePara(int argc, char* argv[])
{
	boost::program_options::options_description opts("tmas-prof options");
	opts.add_options()  
	("help,h",				  "help")
	("pressure-test,p",		  "execute pressure test")
	("prof-measure,m",        "execute profile measurement")
	("sleep-micro-second,s",  po::value<uint32>(&g_cmdline_para.micro_sleep_time), "sleep specified micro-seconds after send a packet. Default: 0.")
	("sleep-nano-second,S",	  po::value<uint32>(&g_cmdline_para.nano_sleep_time),  "sleep specified nano-seconds after send a packet, must be less than 1000. Default: 0.")
	("send-count,c",		  po::value<uint32>(&g_cmdline_para.send_count),	   "total send packet count, 0 means it will send packets infinitly. Default: 0.")
	("concurrent-conn,n",	  po::value<uint32>(&g_cmdline_para.concurrent_conns), "concurrent connection number. Default: 1.");
	
	po::variables_map vm;  
	try  
	{  
		po::store(po::parse_command_line(argc, argv, opts), vm);  
	}  
	catch(boost::program_options::error_with_no_option_name &ex)
	{  
		std::cout << ex.what() << std::endl;
		return false;
	}  
	po::notify(vm); // �������Ľ���洢���ⲿ����

	if (vm.count("help"))
	{
		std::cout << opts << std::endl;
		return false;
	}
	else if (!vm.count("pressure-test") && !vm.count("prof-measure"))
	{
		std::cout << "ERROR: please specify operation type" << std::endl;
		return false;
	}
	else if (vm.count("pressure-test") && vm.count("prof-measure"))
	{
		std::cout << "ERROR: only one operation type can be specified at a time" << std::endl;
		return false;
	}
	else if (vm.count("sleep-micro-second") && vm.count("sleep-nano-second"))
	{
		std::cout << "ERROR: can not specify sleep-micro-second and sleep-nano-second at the same time" << std::endl;
		return false;
	}
	else if (vm.count("sleep-nano-time") && g_cmdline_para.nano_sleep_time >= 1000)
	{
		std::cout << "ERROR: nano-second value must less than 1000" << std::endl;
		return false;
	}

	if (vm.count("pressure-test"))
	{
		g_cmdline_para.pressure_test = true;
	}

	if (vm.count("prof-measure"))
	{
		g_cmdline_para.prof_measure = true;
	}

	return true;
}

void InitGlog()
{
	FLAGS_stderrthreshold = google::INFO;
	FLAGS_minloglevel = 0;
	FLAGS_colorlogtostderr = 1;
	google::InitGoogleLogging("tmas-prof");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintRealTimePressureTestStat()
{
	std::cout << "---------------------------------------" << std::endl;
	std::cout << "Captured packet number     : " << g_prof_stat.captured_pkt_num << std::endl;
	std::cout << "L3 processed packet number : " << g_prof_stat.l3_processed_pkt_num << std::endl;
	std::cout << "L4 processed packet number : " << g_prof_stat.l4_processed_pkt_num << std::endl;
	std::cout << "L7 processed packet number : " << g_prof_stat.l7_processed_pkt_num << std::endl;
	std::cout << "Currrent traffic           : " << g_prof_stat.captured_pkt_num * 155 * 8 / (1000 * 1000) << " Mb/s" << std::endl;

	g_prof_stat.captured_pkt_num = g_prof_stat.l3_processed_pkt_num = g_prof_stat.l4_processed_pkt_num = g_prof_stat.l7_processed_pkt_num = 0;
}

void PrintFinalPressureTestStat()
{
	std::cout << "#############################################" << std::endl;
	std::cout << "Total captured packet number     : " << g_prof_stat.total_captured_pkt_num << std::endl;
	std::cout << "Total L3 processed packet number : " << g_prof_stat.total_l3_processed_pkt_num << std::endl;
	std::cout << "Total L4 processed packet number : " << g_prof_stat.total_l4_processed_pkt_num << std::endl;
	std::cout << "Total L7 processed packet number : " << g_prof_stat.total_l7_processed_pkt_num << std::endl;

	std::cout << "Total dropped packet number      : " << PktDispatcher::GetInstance().get_dropped_pkt_num() << std::endl;

	if (g_prof_stat.total_captured_pkt_num == 0) g_prof_stat.total_captured_pkt_num = 1;
	std::cout << "Total packet loss percent        : " 
		<< (g_prof_stat.total_captured_pkt_num - g_prof_stat.total_l3_processed_pkt_num) * 100 / (float)g_prof_stat.total_captured_pkt_num
		<< " %" << std::endl;

	int64 duration = GetDurationMilliSeconds(g_prof_stat.start_time, g_prof_stat.stop_time);
	if (duration <= 0) duration = 1;

	// 155: 11������ƽ������
	std::cout << "Average traffic                  : " 
		<< g_prof_stat.total_captured_pkt_num * 155 * 8 / duration / 1000
		<< " Mb/s" << std::endl;

	std::cout << "#############################################" << std::endl;
}

bool ConstructPressureTestPktProcChain(PktDispatcher& dispacher)
{
	EthMonitorSP eth_monitor(new EthMonitor());
	if (!eth_monitor->Init())
	{
		LOG(ERROR) << "Fail to init ETH monitor";
		return false;
	}

	IpMonitorSP ip_monitor(new IpMonitor());
	if (!ip_monitor->Init())
	{
		LOG(ERROR) << "Fail to init IP monitor";
		return false;
	}

	L4MonitorSP l4_monitor(new L4Monitor());
	if (!l4_monitor->Init())
	{
		LOG(ERROR) << "Fail to init L4 monitor";
		return false;
	}

	L7MonitorSP l7_monitor(new L7Monitor());
	if (!l7_monitor->Init())
	{
		LOG(ERROR) << "Fail to init L7 monitor";
		return false;
	}

	L3ProfStatProcessorSP l3_stator(new L3ProfStatProcessor());
	L4ProfStatProcessorSP l4_stator(new L4ProfStatProcessor());
	L7ProfStatProcessorSP l7_stator(new L7ProfStatProcessor());

	dispacher.set_pkt_processor(eth_monitor);
	eth_monitor->set_successor(l3_stator);
	l3_stator->set_successor(ip_monitor);
	ip_monitor->set_successor(l4_stator);
	l4_stator->set_successor(l4_monitor);
	l4_monitor->set_successor(l7_stator);
	l7_stator->set_successor(l7_monitor);

	return true;
}

void DoPressureTest()
{
	// ����ʵʱ����ͳ�ƶ�ʱ��
	FreeTimer stat_timer(PrintRealTimePressureTestStat, 1);

	// �������ķַ���
	PktDispatcher& dispatcher = PktDispatcher::GetInstance();

	// �������Ĵ�����
	if (!ConstructPressureTestPktProcChain(dispatcher))
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return;
	}

	// �������Ĳ��������
	PressureTestSimulator simulator(
		boost::bind(&PktDispatcher::ProcessPacket, &dispatcher, _1));

	// ����ѹ���������
	stat_timer.Start();
	dispatcher.Start();
	simulator.Start();

	// ֹͣѹ���������
	stat_timer.Stop();
	dispatcher.Stop();

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	// ��ӡ����ѹ������ͳ�ƽ��
	PrintFinalPressureTestStat(); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PktProcessorSP ConstructProfMeasurePktProcChain()
{
	EthMonitorSP eth_monitor(new EthMonitor());
	if (!eth_monitor->Init())
	{
		LOG(ERROR) << "Fail to init ETH monitor";
		return nullptr;
	}

	IpMonitorSP ip_monitor(new IpMonitor());
	if (!ip_monitor->Init())
	{
		LOG(ERROR) << "Fail to init IP monitor";
		return nullptr;
	}

	L4MonitorSP l4_monitor(new L4Monitor());
	if (!l4_monitor->Init())
	{
		LOG(ERROR) << "Fail to init L4 monitor";
		return nullptr;
	}

	L7MonitorSP l7_monitor(new L7Monitor());
	if (!l7_monitor->Init())
	{
		LOG(ERROR) << "Fail to init L7 monitor";
		return nullptr;
	}

	L3ProfStatProcessorSP l3_stator(new L3ProfStatProcessor());
	L4ProfStatProcessorSP l4_stator(new L4ProfStatProcessor());
	L7ProfStatProcessorSP l7_stator(new L7ProfStatProcessor());

	eth_monitor->set_successor(l3_stator);
	l3_stator->set_successor(ip_monitor);
	ip_monitor->set_successor(l4_stator);
	l4_stator->set_successor(l4_monitor);
	l4_monitor->set_successor(l7_stator);
	l7_stator->set_successor(l7_monitor);

	return eth_monitor;
}

uint32 CalcTotalUsedTime(uint32* stat_data)
{
	uint32 total = 0;
	for (int i = 0; i < 4; i++)
	{
		total += stat_data[i];
	}

	return total;
}

void PrintProfMeasureStatOfOneLayer(uint32 index, uint32* stat_data)
{
	uint64 total = CalcTotalUsedTime(stat_data);

	if (total == 0)
	{
		LOG(ERROR) << "Invalid value";
		return;
	}

	std::cout << std::setw(4) << index << " "
		      << std::setw(6) << stat_data[0] << "(" << std::setw(2) << stat_data[0] * 100 / total << "%)"
			  << std::setw(6) << stat_data[1] << "(" << std::setw(2) << stat_data[1] * 100 / total << "%)"
			  << std::setw(6) << stat_data[2] << "(" << std::setw(2) << stat_data[2] * 100 / total << "%)"
			  << std::setw(6) << stat_data[3] << "(" << std::setw(2) << stat_data[3] * 100 / total << "%)"
			  << std::endl;
}

void PrintProfMeasureFinalStat()
{
	std::cout << "==================================================" << std::endl;
	std::cout << "Index |   ->L3   |     L3   |     L4   |     L7   " << std::endl;
	std::cout << "==================================================" << std::endl;
	
	// ˳���ӡ11�����ĵ�ͳ����Ϣ
	for (int i = 0; i < 11; i++)
	{
		PrintProfMeasureStatOfOneLayer(i + 1, g_prof_stat.time_use_of_layer[i]);
	}

	std::cout << "==================================================" << std::endl;
}

void DoProfMeasure()
{
	PktProcessorSP processor = ConstructProfMeasurePktProcChain();
	if (!processor)
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return;
	}

	// �������Ĳ��������
	ProfMeasureSimulator simulator(processor);

	// ��������
	simulator.Start();

	// ��ӡ���Խ��
	PrintProfMeasureFinalStat();
}

int main(int argc, char* argv[])
{	
	GlobalDataInit(); // ȫ�����ݳ�ʼ��

	InitGlog();	// ��ʼ��glog

	// ���������в���
	if (!ParseCmdLinePara(argc, argv)) return -1;

	// ���������ļ�
    TmasConfigParser::GetInstance().Init();

	if (g_cmdline_para.pressure_test)
	{
		DoPressureTest();
	}
	else if (g_cmdline_para.prof_measure)
	{
		DoProfMeasure();
	}
	
    return 0;
}
