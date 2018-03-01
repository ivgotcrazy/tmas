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
#include "ip_monitor.hpp"
#include "l4_monitor.hpp"
#include "l7_monitor.hpp"
#include "database_recorder.hpp"
#include "tmas_assert.hpp"
#include "pkt_capturer.hpp"

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
 * ��  ��: �������Ĵ�����
 * ��  ��: [out] dispatcher ���ķַ���
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��28��
 *   ���� teck_zhou
 *   ���� ����
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
	// ��ʼ��glog
	if (!InitGlog())
	{
		perror("Fail to init glog");
		return -1;
	}

	// ��ʼ�������ļ�������
	TmasConfigParser::GetInstance().Init();

	// ��ʼ�����ݿ�
	if(!DatabaseRecorder::GetInstance().Init())
	{
		LOG(ERROR) << "Fail to init database recorder";
		return -1;
	}

	// ��ʼ�����ķַ���
	PktDispatcher& dispatcher = PktDispatcher::GetInstance();
	
	// ��ʼ�����Ĳ�����
	PktCapturer pkt_capturer(boost::bind(&PktDispatcher::ProcessPacket, &dispatcher, _1));
	if (!pkt_capturer.Init())
	{
		LOG(ERROR) << "Fail to init packet capturer";
		return -1;
	}

	// �������Ĵ�����
    if (!ConstructPktProcChain(dispatcher))
	{
		LOG(ERROR) << "Fail to construct packet process chain";
		return -1;
	}

	// �������Ĳ���
	dispatcher.Start();
	pkt_capturer.Start();

	//--- ���߳�˯�� ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	return 0;
}
