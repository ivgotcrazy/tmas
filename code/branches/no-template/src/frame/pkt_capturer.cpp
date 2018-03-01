/*#############################################################################
 * 文件名   : pkt_capturer.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2013年12月31日
 * 文件描述 : PktCapturer类声明
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "pkt_capturer.hpp"

#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "mem_buf_pool.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_cfg.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktCapturer::PktCapturer(const PktHandler& handler)
	: pkt_handler_(handler), stop_flag_(false)
{
	TMAS_ASSERT(pkt_handler_);
}

/*-----------------------------------------------------------------------------
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktCapturer::~PktCapturer()
{
	stop_flag_ = true;

	for (ThreadSP& t : capture_threads_)
	{
		t->join();
	}

	for (pcap_t* session : capture_sessions_)
	{
		pcap_close(session);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取过滤规则
 * 参  数: 
 * 返回值: 过滤规则
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
std::string PktCapturer::GetPktFilter() 
{ 
	std::string filter_str;
	GET_TMAS_CONFIG_STR("global.common.capture-filter", filter_str);

	DLOG(INFO) << "Capture filter | " << filter_str;

	return filter_str;
}

/*-----------------------------------------------------------------------------
 * 描  述: 打开捕获网卡
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool PktCapturer::OpenCaptureInterface()
{
	// 从配置文件读取抓包网卡列表
	std::string interface_str;
    GET_TMAS_CONFIG_STR("global.common.capture-interface", interface_str);	

	if (interface_str.empty())
	{
		LOG(ERROR) << "Empty capture interface";
		return false;
	}

	std::vector<std::string> capture_interfaces = SplitStr(interface_str, ' ');
	TMAS_ASSERT(!capture_interfaces.empty());

	// 打开抓包网卡

	char errbuf[PCAP_ERRBUF_SIZE];
	for (std::string& capture_interface : capture_interfaces)
	{
		pcap_t* session = pcap_open_live(capture_interface.c_str(),	// name of the device
										 PCAP_PKT_MAX_SIZE,			// portion of the packet to capture
										 1,							// promiscuous mode (nonzero means promiscuous)
										 1,							// read timeout
										 errbuf);					// error buffer
		if (!session)
		{
			LOG(ERROR) << "Fail to open interface " << capture_interface
				       << " | " << errbuf;
			return false;
		}

		capture_sessions_.push_back(session);

        DLOG(INFO) << "Open capture interface " << capture_interface;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置捕获规则
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool PktCapturer::SetCaptureFilter()
{
	pcap_direction_t direction = PCAP_D_INOUT; // 固定只抓入方向的报文

	for (pcap_t* session : capture_sessions_)
	{
		if (-1 == pcap_setdirection(session, direction))
		{
			LOG(ERROR) << "Fail to capture direction | " << pcap_geterr(session);
			return false;
		}

		int datalink_type = pcap_datalink(session);
		if (-1 == datalink_type)
		{
			LOG(ERROR) << "Fail to get datalink type | " << pcap_geterr(session);
			return false;
		}

		// 编译过滤规则
		bpf_program compiled_filter;
		if (-1 == pcap_compile_nopcap(PCAP_PKT_MAX_SIZE, 
									  datalink_type, 
									  &compiled_filter, 
									  GetPktFilter().c_str(), 
									  1, 
									  0xFFFFFFFF))
		{
			LOG(ERROR) << "Fail to compile pcap filter | " << pcap_geterr(session);
			return false;
		}

		// 设置过滤规则
		if (-1 == pcap_setfilter(session, &compiled_filter))
		{
			LOG(ERROR) << "Fail to set capture filter " << pcap_geterr(session);
			return false;
		}
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool PktCapturer::Init()
{
	if (!OpenCaptureInterface())
	{
		LOG(ERROR) << "Fail to open capture interface";
		return false;
	}

	if (!SetCaptureFilter())
	{
		LOG(ERROR) << "Fail to set capture filter";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 启动抓包
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktCapturer::Start()
{
	for (pcap_t* session : capture_sessions_)
	{
		capture_threads_.push_back(ThreadSP(new boost::thread(
			boost::bind(&PktCapturer::PktCaptureFunc, this, session))));
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 停止抓包
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktCapturer::Stop()
{
	stop_flag_ = true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文捕获线程处理函数
 * 参  数: [in] session 抓包网卡
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktCapturer::PktCaptureFunc(pcap_t* session)
{
	int ret;
	pcap_pkthdr* header = nullptr;
	const u_char* packet = nullptr;
	PktEntry pkt_entry;

	while (!stop_flag_)
	{
		ret = pcap_next_ex(session, &header, &packet);
		if (ret < 0)
		{
			LOG(ERROR) << "Fail to read packets | " << pcap_geterr(session);
			return;
		}

		// Timeout elapsed
		if (ret == 0) continue; 

		// that means the packet has been truncated or it's a malformed packet
		if (header->caplen != header->len || header->len < PCAP_PKT_MIN_SIZE)
		{
			continue;
		}

		if (header->len > 1600)
		{
			DLOG(WARNING) << "Unexpected packet len: "  << header->len;
			continue;
		}

		pkt_entry.buf = reinterpret_cast<char*>(const_cast<u_char*>(packet));
		pkt_entry.len = header->len;

		// 将报文扔到PktDispatcher的处理队列立即返回
		pkt_handler_(pkt_entry); 
	}
}

}