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
#include "ip_monitor.hpp"
#include "l4_monitor.hpp"
#include "l7_monitor.hpp"
#include "database_recorder.hpp"
#include "tmas_assert.hpp"
#include "pkt_capturer.hpp"

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
 * 描  述: 构建报文处理链
 * 参  数: [out] dispatcher 报文分发器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月28日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ConstructPktProcChain(PktDispatcher& dispacher)
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

	dispacher.set_pkt_processor(eth_monitor);
	eth_monitor->set_successor(ip_monitor);
	ip_monitor->set_successor(l4_monitor);
	l4_monitor->set_successor(l7_monitor);

	return true;
}

int main(int argc, char* argv[])
{
	// 初始化glog
	if (!InitGlog())
	{
		perror("Fail to init glog");
		return -1;
	}

	// 初始化配置文件解析器
	TmasConfigParser::GetInstance().Init();

	// 初始化数据库
	if(!DatabaseRecorder::GetInstance().Init())
	{
		LOG(ERROR) << "Fail to init database recorder";
		return -1;
	}

	// 初始化报文分发器
	PktDispatcher& dispatcher = PktDispatcher::GetInstance();
	
	// 初始化报文捕获器
	PktCapturer pkt_capturer(boost::bind(&PktDispatcher::ProcessPacket, &dispatcher, _1));
	if (!pkt_capturer.Init())
	{
		LOG(ERROR) << "Fail to init packet capturer";
		return -1;
	}

	// 构建报文处理链
    if (!ConstructPktProcChain(dispatcher))
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return -1;
	}

	// 启动报文捕获
	dispatcher.Start();
	pkt_capturer.Start();

	//--- 主线程睡眠 ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	return 0;
}
