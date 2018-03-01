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
#include "tmas_assert.hpp"
#include "tmas_typedef.hpp"
#include "pkt_receiver.hpp"
#include "pkt_distributor.hpp"
#include "cpu_core_manager.hpp"
#include "pcap_pkt_capturer.hpp"

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
 * ��  ��: �������Ľ�����
 * ��  ��: [out] pkt_receiver ���Ľ���������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool ConstructPktReceivers(uint8 distributor_count, PktReceiverVec& pkt_receivers)
{
	uint32 receiver_count;
	GET_TMAS_CONFIG_INT("global.netmap.packet-process-thread-count", receiver_count);

	// ����ÿ��PktReceiver���ᴴ��һ���̣߳���ˣ���������̫��
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
 * ��  ��: �������ķַ���
 * ��  ��: [in] receivers ���Ľ�����
 *         [out] distributor ���ķַ���
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool ConstructPktDistributors(const std::vector<std::string>& interfaces, 
	                          PktReceiverVec& receivers, 
							  PktDistributorVec& distributors)
{
	// �������ͬʱץ��������Ҫ��������ַ���������ַ��������ķַ���ͬһ������
	// ��ʱ���Խ������Ļ��������ʿ϶���Ҫ������Ϊ���������ּ�������������ÿ��
	// �ַ�����������������һ����Ӧ�ı��Ļ���������������ַ��������ķַ���ͬһ
    // ��������ʱ�����Կ��Է����Լ������Ļ�������Ϊ�˱�֤���ַ��ʵĹ淶�Ժ���ȷ
	// �ԣ���ҪΪÿ���ַ�������һ�������������������0��ʼ������š�
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
 * ��  ��: ȫ�ֳ�ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool GlobalInitialize()
{
	// ��ʼ��glog
	if (!InitGlog())
	{
		perror("Fail to initialize glog");
		return false;
	}

	//FLAGS_minloglevel = 1;

	// ��ʼ�������ļ�������
	try
	{
		TmasConfigParser::GetInstance().Init();
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << e.what();
		return false;
	}

	// ��ʼ��CPU���Ĺ�����
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

	// �������Ľ�����
	PktReceiverVec pkt_receivers;
	if (!ConstructPktReceivers(interfaces.size(), pkt_receivers))
	{
		LOG(ERROR) << "Fail to construct packet receiver";
		return false;
	}

	// �������ķַ���
	PktDistributorVec pkt_distributors;
	if (!ConstructPktDistributors(interfaces, pkt_receivers, pkt_distributors))
	{
		LOG(ERROR) << "Fail to construct packet distributor";
		return false;
	}

	// ���������Ľ�����
	for (PktReceiverSP& receiver : pkt_receivers)
	{
		receiver->Start();
	}

	// Ȼ���������ķַ���
	for (PktDistributorSP& distributor : pkt_distributors)
	{
		distributor->Start();
	}

	//--- ���߳�˯�� ---

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

	//--- ���߳�˯�� ---

	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
}

int main(int argc, char* argv[])
{
	// ȫ�ֳ�ʼ��
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
