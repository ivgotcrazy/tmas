/*#############################################################################
 * �ļ���   : pcap_pkt_capturer.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2013��12��31��
 * �ļ����� : PcapPktCapturer��ʵ��
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "pcap_pkt_capturer.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_cfg.hpp"
#include "file_logger.hpp"
#include "database_logger.hpp"

#include "eth_monitor.hpp"
#include "ipv4_monitor.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "http_monitor.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2013��11��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PcapPktCapturer::PcapPktCapturer() 
	: stop_flag_(false)
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PcapPktCapturer::~PcapPktCapturer()
{
	if (!stop_flag_) Stop();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ���˹���
 * ��  ��: 
 * ����ֵ: ���˹���
 * ��  ��:
 *   ʱ�� 2014��01��02��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
std::string PcapPktCapturer::GetPktFilter() 
{ 
	std::string filter_str;
	GET_TMAS_CONFIG_STR("global.capture.capture-filter", filter_str);

	DLOG(INFO) << "Capture filter | " << filter_str;

	return filter_str;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �򿪲�������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PcapPktCapturer::OpenCaptureInterface()
{
	// �������ļ���ȡץ�������б�
	std::string interface_str;
    GET_TMAS_CONFIG_STR("global.capture.capture-interface", interface_str);	

	if (interface_str.empty())
	{
		LOG(ERROR) << "Empty capture interface";
		return false;
	}

	std::vector<std::string> capture_interfaces = SplitStr(interface_str, ' ');
	if (capture_interfaces.empty())
	{
		LOG(ERROR) << "No capture interface specified";
		return false;
	}
	
	if (capture_interfaces.size() > 1)
	{
		LOG(ERROR) << "Can open only one interface in pcap capture mode";
		return false;
	}

	// ��ץ������

	char errbuf[PCAP_ERRBUF_SIZE];
	for (std::string& capture_interface : capture_interfaces)
	{
		pcap_t* session = pcap_open_live(
			capture_interface.c_str(),	// name of the device
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

        DLOG(INFO) << "Open capture interface " << capture_interface << " successfully";
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ò������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PcapPktCapturer::SetCaptureFilter()
{
	pcap_direction_t direction = PCAP_D_INOUT; // �̶�ֻץ�뷽��ı���

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

		// ������˹���
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

		// ���ù��˹���
		if (-1 == pcap_setfilter(session, &compiled_filter))
		{
			LOG(ERROR) << "Fail to set capture filter " << pcap_geterr(session);
			return false;
		}
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ò������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PcapPktCapturer::ConstructPktProcessor()
{
	// �ȴ������ݼ�¼��
	LoggerSP logger;

	std::string log_type;
	GET_TMAS_CONFIG_STR("global.common.log-record-type", log_type);

	if (log_type == "file")
	{
		logger.reset(new FileLogger());
	}
	else if (log_type == "db")
	{
		logger.reset(new DatabaseLogger());
	}
	else
	{
		LOG(ERROR) << "Invalid log type " << log_type;
		return false;
	}

	if (!logger->Init())
	{
		LOG(ERROR) << "Fail to initialize logger";
		return false;
	}

	//======================= L2 ============================

	EthMonitorTypeSP eth_monitor(new EthMonitorType);
	if (!eth_monitor->Init())
	{
		LOG(ERROR) << "Fail to init eth monitor";
		return false;
	}

	//======================= L3 ============================

	Ipv4MonitorTypeSP ipv4_monitor(new Ipv4MonitorType);
	if (!ipv4_monitor->Init())
	{
		LOG(ERROR) << "Fail to init ip monitor";
		return false;
	}

	//======================= L4 ============================

	TcpMonitorTypeSP tcp_monitor(new TcpMonitorType(logger));
	if (!tcp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init tcp monitor";
		return false;
	}

	UdpMonitorTypeSP udp_monitor(new UdpMonitorType);
	if (!udp_monitor->Init())
	{
		LOG(ERROR) << "Fail to init udp monitor";
		return false;
	}

	//======================= L7 ============================

	HttpMonitorTypeSP http_monitor(new HttpMonitorType(logger));
	if (!http_monitor->Init())
	{
		LOG(ERROR) << "Fail to init http monitor";
		return false;
	}

	//=== ���촦����

	eth_monitor->SetSuccProcessor(ipv4_monitor);

	ipv4_monitor->SetSuccProcessor(tcp_monitor);

	tcp_monitor->SetNextProcessor(udp_monitor);

	tcp_monitor->SetSuccProcessor(http_monitor);

	pkt_processor_ = eth_monitor;

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PcapPktCapturer::Init()
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

	if (!ConstructPktProcessor())
	{
		LOG(ERROR) << "Fail to construct packet processor";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����ץ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PcapPktCapturer::Start()
{
	for (pcap_t* session : capture_sessions_)
	{
		capture_threads_.push_back(ThreadSP(new boost::thread(
			boost::bind(&PcapPktCapturer::PktCaptureFunc, this, session))));
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ֹͣץ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PcapPktCapturer::Stop()
{
	stop_flag_ = true;

	// ֹͣץ��
	for (ThreadSP& t : capture_threads_)
	{
		t->interrupt();
		t->join();
	}

	// �ر�pcap
	for (pcap_t* session : capture_sessions_)
	{
		pcap_close(session);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĳ����̴߳�����
 * ��  ��: [in] session ץ������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2013��11��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PcapPktCapturer::PktCaptureFunc(pcap_t* session)
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

		if (header->len > PCAP_PKT_MAX_SIZE)
		{
			LOG(WARNING) << "Unexpected packet len: "  << header->len;
			continue;
		}

		// ������
		PktInfo pkt_info;
		pkt_info.pkt.buf = reinterpret_cast<char*>(const_cast<u_char*>(packet));
		pkt_info.pkt.len = header->len;
		pkt_info.arrive  = GetMicroSecond();

		pkt_processor_->ProcessMsg(MSG_PKT, (void*)(&pkt_info));
	}
}

}