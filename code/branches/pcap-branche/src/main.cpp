/*#############################################################################
 * 文件名   : main.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2013年12月31日
 * 文件描述 : main入口
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <string>
#include <glog/logging.h>

#include "tmas_config_parser.hpp"
#include "pkt_dispatcher.hpp"
#include "eth_monitor.hpp"
#include "ipv4_monitor.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "http_monitor.hpp"
#include "database_recorder.hpp"
#include "tmas_assert.hpp"
#include "pkt_capturer.hpp"
#include "tmas_typedef.hpp"

using namespace BroadInter;

/*-----------------------------------------------------------------------------
 * 描  述: 设置glog的log保存路径
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool SetGlogSavePath()
{
	std::string log_path_str = "/tmp";

	// 如果log路径不存在，则先创建路径
	fs::path log_path(log_path_str);
	if (!fs::exists(log_path))
	{
		if (!fs::create_directory(log_path))
		{
			LOG(ERROR) << "Fail to create log path" << log_path;
			return false;
		}
	}

	FLAGS_log_dir = log_path_str;

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置glog的log级别
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool SetGlogLogLevel()
{
	uint32 log_level = 0;

	if (log_level > 3) return false;

	FLAGS_minloglevel = log_level;

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化glog
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool InitGlog()
{
	if (!SetGlogSavePath()) return false;

	if (!SetGlogLogLevel()) return false;

	FLAGS_stderrthreshold = google::INFO;
	FLAGS_colorlogtostderr = 1;

	google::InitGoogleLogging("rcs");

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 构建处理链
 * 参  数: 
 * 返回值: 处理链
 * 修  改:
 *   时间 2014年02月28日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
EthMonitorTypeSP ConstructProcChain()
{
	//======================= L2 ============================

	EthMonitorTypeSP eth_monitor(new EthMonitorType);
	if (!eth_monitor->Init())
	{
		LOG(ERROR) << "Fail to init eth monitor";
		return nullptr;
	}

	//======================= L3 ============================

	Ipv4MonitorTypeSP ipv4_monitor(new Ipv4MonitorType);
	if (!ipv4_monitor->Init())
	{
		LOG(ERROR) << "Fail to init ip monitor";
		return nullptr;
	}

	//======================= L4 ============================

	TcpMonitorTypeSP tcp_monitor(new TcpMonitorType);
	if (!tcp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init tcp monitor";
		return nullptr;
	}

	UdpMonitorTypeSP udp_monitor(new UdpMonitorType);
	if (!udp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init udp monitor";
		return nullptr;
	}

	//======================= L7 ============================

	HttpMonitorTypeSP http_monitor(new HttpMonitorType);
	if (!http_monitor->Init())
	{
		LOG(ERROR) << "Fail to init http monitor";
		return nullptr;
	}

	//=== 构造处理链

	eth_monitor->SetSuccProcessor(ipv4_monitor);

	ipv4_monitor->SetSuccProcessor(tcp_monitor);

	tcp_monitor->SetNextProcessor(udp_monitor);

	tcp_monitor->SetSuccProcessor(http_monitor);

	return eth_monitor;
}

int main(int argc, char* argv[])
{
	// 初始化glog
	if (!InitGlog())
	{
		perror("Fail to init glog");
		return -1;
	}

	//FLAGS_minloglevel = 1;

	// 初始化配置文件解析器
	TmasConfigParser::GetInstance().Init();

	// 初始化数据库
	if(!DatabaseRecorder::GetInstance().Init())
	{
		LOG(ERROR) << "Fail to init database recorder";
		return -1;
	}

	// 初始化报文分发器
	PktDispatcherTypeSP dispatcher = PktDispatcherType::GetInstance();
	TMAS_ASSERT(dispatcher);
	if (!dispatcher->Init())
	{
		LOG(ERROR) << "Fail to init packet dispatcher";
		return -1;
	}
	
	// 初始化报文捕获器 
	PktCapturerSP capturer(new PktCapturer(dispatcher));
	if (!capturer->Init())
	{
		LOG(ERROR) << "Fail to init packet capturer";
		return -1;
	}

	// 构建报文处理链
	EthMonitorTypeSP eth_monitor = ConstructProcChain();
	if (!eth_monitor)
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return -1;
	}

	// 将报文捕获器、报文分发器和处理链组装起来
	dispatcher->SetPktProcessor(eth_monitor);

	// 先启动报文分发器
	dispatcher->Start();

	// 再启动报文捕获器
	capturer->Start();

	//--- 主线程睡眠 ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	return 0;
}
