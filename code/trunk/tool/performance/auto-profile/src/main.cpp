#include <string>
#include <glog/logging.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>  

#include "tmas_config_parser.hpp"
#include "eth_monitor.hpp"
#include "ipv4_monitor.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "http_monitor.hpp"
#include "pkt_dispatcher.hpp"
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
	po::notify(vm); // 将解析的结果存储到外部变量

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

EthMonitorTestTypeSP ConstructProfProcChain()
{
	EthMonitorTestTypeSP eth_monitor(new EthMonitorTestType());
	if (!eth_monitor->Init())
	{
		LOG(ERROR) << "Fail to init eth monitor";
		return false;
	}

	Ipv4MonitorTestTypeSP ipv4_monitor(new Ipv4MonitorTestType());
	if (!ipv4_monitor->Init())
	{
		LOG(ERROR) << "Fail to init ip monitor";
		return false;
	}

	TcpMonitorTestTypeSP tcp_monitor(new TcpMonitorTestType());
	if (!tcp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init tcp monitor";
		return false;
	}

	UdpMonitorTestTypeSP udp_monitor(new UdpMonitorTestType());
	if (!udp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init udp monitor";
		return false;
	}

	HttpMonitorTestTypeSP http_monitor(new HttpMonitorTestType());
	if (!http_monitor->Init())
	{
		LOG(ERROR) << "Fail to init http monitor";
		return false;
	}

	L3ProfStatProcessorTypeSP l3_stator(new L3ProfStatProcessorType());
	L4ProfStatProcessorTypeSP l4_stator(new L4ProfStatProcessorType());

	eth_monitor->SetSuccProcessor(l3_stator);

	l3_stator->SetSuccProcessor(ipv4_monitor);

	ipv4_monitor->SetSuccProcessor(l4_stator);

	l4_stator->SetSuccProcessor(tcp_monitor);

	tcp_monitor->SetSuccProcessor(http_monitor);

	tcp_monitor->SetNextProcessor(udp_monitor);

	return eth_monitor;
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

	std::cout << "Total dropped packet number      : " << PktDispatcherTestType::GetInstance()->get_dropped_pkt_num() << std::endl;

	if (g_prof_stat.total_captured_pkt_num == 0) g_prof_stat.total_captured_pkt_num = 1;
	std::cout << "Total packet loss percent        : " 
		<< (g_prof_stat.total_captured_pkt_num - g_prof_stat.total_l3_processed_pkt_num) * 100 / (float)g_prof_stat.total_captured_pkt_num
		<< " %" << std::endl;

	int64 duration = GetDurationMilliSeconds(g_prof_stat.start_time, g_prof_stat.stop_time);
	if (duration <= 0) duration = 1;

	// 155: 11个报文平均长度
	std::cout << "Average traffic                  : " 
		<< g_prof_stat.total_captured_pkt_num * 155 * 8 / duration / 1000
		<< " Mb/s" << std::endl;

	std::cout << "#############################################" << std::endl;
}

void DoPressureTest()
{
	// 启动实时性能统计定时器
	FreeTimer stat_timer(PrintRealTimePressureTestStat, 1);

	// 创建报文分发器
	PktDispatcherTestTypeSP dispatcher = PktDispatcherTestType::GetInstance();
	if (!dispatcher->Init())
	{
		LOG(ERROR) << "Fail to init packet dispaturer";
		return;
	}

	// 构建报文处理链
	EthMonitorTestTypeSP eth_monitor = ConstructProfProcChain();
	if (!eth_monitor)
	{
		LOG(ERROR) << "Fail to construct eth monitor";
		return;
	}

	dispatcher->SetPktProcessor(eth_monitor);

	// 创建报文捕获仿真器
	PressureTestSimulator simulator(dispatcher);

	// 启动压力仿真测试
	stat_timer.Start();
	dispatcher->Start();
	simulator.Start();

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	// 停止压力仿真测试
	stat_timer.Stop();
	dispatcher->Stop();

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	// 打印最终压力测试统计结果
	PrintFinalPressureTestStat(); 
}

///////////////////////////////////////////////////////////////////////////////

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
	
	// 顺序打印11个报文的统计信息
	for (int i = 0; i < 11; i++)
	{
		PrintProfMeasureStatOfOneLayer(i + 1, g_prof_stat.time_use_of_layer[i]);
	}

	std::cout << "==================================================" << std::endl;
}

void DoProfMeasure()
{
	EthMonitorTestTypeSP eth_monitor = ConstructProfProcChain();
	if (!eth_monitor)
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return;
	}

	// 创建报文捕获仿真器
	ProfMeasureSimulator simulator(eth_monitor);

	// 启动仿真
	simulator.Start();

	// 打印测试结果
	PrintProfMeasureFinalStat();
}

int main(int argc, char* argv[])
{	
	GlobalDataInit(); // 全局数据初始化

	InitGlog();	// 初始化glog

	// 解析命令行参数
	if (!ParseCmdLinePara(argc, argv)) return -1;

	// 解析配置文件
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
