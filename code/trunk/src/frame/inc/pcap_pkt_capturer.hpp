/*#############################################################################
 * 文件名   : pcap_pkt_capturer.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2013年12月31日
 * 文件描述 : PcapPktCapturer类声明
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PCAP_PKT_CAPTURER
#define BROADINTER_PCAP_PKT_CAPTURER

#include <string>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "pcap.h"
#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述：报文捕获器
 * 作者：teck_zhou
 * 时间：2013年11月13日
 *****************************************************************************/
class PcapPktCapturer : public boost::noncopyable
{
public:
	PcapPktCapturer();
	~PcapPktCapturer();

	bool Init();

	void Start();

	void Stop();

private:
	std::string GetPktFilter();

	void PktCaptureFunc(pcap_t* session);

	bool OpenCaptureInterface();

	bool SetCaptureFilter();

	bool ConstructPktProcessor();

private:
	EthMonitorTypeSP pkt_processor_;

	// 每个网卡使用一个线程抓包
	std::vector<ThreadSP> capture_threads_;

	// 每个网卡对应一个会话
	std::vector<pcap_t*> capture_sessions_;

	bool stop_flag_;
};

}

#endif