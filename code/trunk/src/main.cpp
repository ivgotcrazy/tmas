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
#include "tmas_assert.hpp"
#include "tmas_typedef.hpp"
#include "pkt_receiver.hpp"
#include "pkt_distributor.hpp"
#include "cpu_core_manager.hpp"
#include "pcap_pkt_capturer.hpp"

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
 * 描  述: 创建报文接收器
 * 参  数: [out] pkt_receiver 报文接收器容器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ConstructPktReceivers(uint8 distributor_count, PktReceiverVec& pkt_receivers)
{
	uint32 receiver_count;
	GET_TMAS_CONFIG_INT("global.netmap.packet-process-thread-count", receiver_count);

	// 由于每个PktReceiver都会创建一个线程，因此，不能配置太多
	if (receiver_count == 0 || receiver_count > 1024)
	{
		LOG(ERROR) << "Invalid packet-process-thread-count : " << receiver_count;
		return false;
	}

	for (uint8 i = 0; i < receiver_count; i++)
	{
		PktReceiverSP receiver(new PktReceiver(i, distributor_count));
		if (!receiver->Init())
		{
			pkt_receivers.clear();

			LOG(ERROR) << "Fail to initialize packet receiver";
			return false;
		}

		pkt_receivers.push_back(receiver);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 创建报文分发器
 * 参  数: [in] receivers 报文接收器
 *         [out] distributor 报文分发器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ConstructPktDistributors(const std::vector<std::string>& interfaces, 
	                          PktReceiverVec& receivers, 
							  PktDistributorVec& distributors)
{
	// 多个网卡同时抓包，则需要创建多个分发器，多个分发器将报文分发到同一个接收
	// 器时，对接收器的缓冲区访问肯定需要加锁。为了消除这种加锁开销，对于每个
	// 分发器，接收器都创建一个对应的报文缓冲区，这样多个分发器将报文分发到同一
    // 个接收器时，各自可以访问自己独立的缓冲区。为了保证这种访问的规范性和正确
	// 性，需要为每个分发器分配一个索引，此索引必须从0开始连续编号。
	uint8 index = 0; 
	for (const std::string& device : interfaces)
	{
		PktDistributorSP distributor(new PktDistributor(index, device, receivers));
		if (!distributor->Init())
		{
			distributors.clear();

			LOG(ERROR) << "Fail to initialize packet distributor";
			return false;
		}

		distributors.push_back(distributor);
		index++;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 全局初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool GlobalInitialize()
{
	// 初始化glog
	if (!InitGlog())
	{
		perror("Fail to initialize glog");
		return false;
	}

	//FLAGS_minloglevel = 1;

	// 初始化配置文件解析器
	try
	{
		TmasConfigParser::GetInstance().Init();
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << e.what();
		return false;
	}

	// 初始化CPU核心管理器
	if (!CpuCoreManager::GetInstance().Init())
	{
		LOG(ERROR) << "Fail to initialize database recorder";
		return false;
	}

	return true;
}

bool StartNetmapCapturer()
{
	std::string interface_str;
	GET_TMAS_CONFIG_STR("global.capture.capture-interface", interface_str);

	std::vector<std::string> interfaces = SplitStr(interface_str, ' ');
	if (interfaces.empty())
	{
		LOG(ERROR) << "Empty capture interface";
		return false;
	}

	// 创建报文接收器
	PktReceiverVec pkt_receivers;
	if (!ConstructPktReceivers(interfaces.size(), pkt_receivers))
	{
		LOG(ERROR) << "Fail to construct packet receiver";
		return false;
	}

	// 创建报文分发器
	PktDistributorVec pkt_distributors;
	if (!ConstructPktDistributors(interfaces, pkt_receivers, pkt_distributors))
	{
		LOG(ERROR) << "Fail to construct packet distributor";
		return false;
	}

	// 先启动报文接收器
	for (PktReceiverSP& receiver : pkt_receivers)
	{
		receiver->Start();
	}

	// 然后启动报文分发器
	for (PktDistributorSP& distributor : pkt_distributors)
	{
		distributor->Start();
	}

	//--- 主线程睡眠 ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	return true;
}

bool StartPcapCapturer()
{
	std::string interface_str;
	GET_TMAS_CONFIG_STR("global.capture.capture-interface", interface_str);

	PcapPktCapturer pkt_capturer;
	
	if (!pkt_capturer.Init())
	{
		LOG(ERROR) << "Fail to initialize pcap capturer";
		return false;
	}

	pkt_capturer.Start();

	//--- 主线程睡眠 ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
}

int main(int argc, char* argv[])
{
	// 全局初始化
	if (!GlobalInitialize()) return -1;

	std::string capture_type;
	GET_TMAS_CONFIG_STR("global.capture.packet-capture-type", capture_type);

	if (capture_type == "pcap")
	{
		if (!StartPcapCapturer())
		{
			LOG(ERROR) << "Fail to start pcap capturer";
			return -1;
		}
	}
	else if (capture_type == "netmap")
	{
		if (!StartNetmapCapturer())
		{
			LOG(ERROR) << "Fail to start netmap capturer";
			return -1;
		}
	}
	else 
	{
		LOG(ERROR) << "Invalid capture type | " << capture_type;
		return -1;
	}

	return 0;
}
