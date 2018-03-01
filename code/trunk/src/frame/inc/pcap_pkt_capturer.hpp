/*#############################################################################
 * �ļ���   : pcap_pkt_capturer.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2013��12��31��
 * �ļ����� : PcapPktCapturer������
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
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
 * ���������Ĳ�����
 * ���ߣ�teck_zhou
 * ʱ�䣺2013��11��13��
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

	// ÿ������ʹ��һ���߳�ץ��
	std::vector<ThreadSP> capture_threads_;

	// ÿ��������Ӧһ���Ự
	std::vector<pcap_t*> capture_sessions_;

	bool stop_flag_;
};

}

#endif