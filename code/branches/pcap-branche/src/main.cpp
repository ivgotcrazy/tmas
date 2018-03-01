/*#############################################################################
 * �ļ���   : main.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2013��12��31��
 * �ļ����� : main���
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
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
 * ��  ��: ����glog��log����·��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool SetGlogSavePath()
{
	std::string log_path_str = "/tmp";

	// ���log·�������ڣ����ȴ���·��
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
 * ��  ��: ����glog��log����
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool SetGlogLogLevel()
{
	uint32 log_level = 0;

	if (log_level > 3) return false;

	FLAGS_minloglevel = log_level;

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��glog
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����������
 * ��  ��: 
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��02��28��
 *   ���� teck_zhou
 *   ���� ����
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

	//=== ���촦����

	eth_monitor->SetSuccProcessor(ipv4_monitor);

	ipv4_monitor->SetSuccProcessor(tcp_monitor);

	tcp_monitor->SetNextProcessor(udp_monitor);

	tcp_monitor->SetSuccProcessor(http_monitor);

	return eth_monitor;
}

int main(int argc, char* argv[])
{
	// ��ʼ��glog
	if (!InitGlog())
	{
		perror("Fail to init glog");
		return -1;
	}

	//FLAGS_minloglevel = 1;

	// ��ʼ�������ļ�������
	TmasConfigParser::GetInstance().Init();

	// ��ʼ�����ݿ�
	if(!DatabaseRecorder::GetInstance().Init())
	{
		LOG(ERROR) << "Fail to init database recorder";
		return -1;
	}

	// ��ʼ�����ķַ���
	PktDispatcherTypeSP dispatcher = PktDispatcherType::GetInstance();
	TMAS_ASSERT(dispatcher);
	if (!dispatcher->Init())
	{
		LOG(ERROR) << "Fail to init packet dispatcher";
		return -1;
	}
	
	// ��ʼ�����Ĳ����� 
	PktCapturerSP capturer(new PktCapturer(dispatcher));
	if (!capturer->Init())
	{
		LOG(ERROR) << "Fail to init packet capturer";
		return -1;
	}

	// �������Ĵ�����
	EthMonitorTypeSP eth_monitor = ConstructProcChain();
	if (!eth_monitor)
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return -1;
	}

	// �����Ĳ����������ķַ����ʹ�������װ����
	dispatcher->SetPktProcessor(eth_monitor);

	// ���������ķַ���
	dispatcher->Start();

	// ���������Ĳ�����
	capturer->Start();

	//--- ���߳�˯�� ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	return 0;
}
