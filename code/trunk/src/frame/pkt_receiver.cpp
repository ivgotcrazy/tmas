/*#############################################################################
 * �ļ���   : pkt_receiver.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��19��
 * �ļ����� : PktReceiver������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tmas_config_parser.hpp"
#include "cpu_core_manager.hpp"
#include "pkt_receiver.hpp"
#include "circular_fifo.hpp"
#include "eth_monitor.hpp"
#include "ipv4_monitor.hpp"
#include "tcp_monitor.hpp"
#include "udp_monitor.hpp"
#include "http_monitor.hpp"
#include "tmas_util.hpp"
#include "file_logger.hpp"
#include "database_logger.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktReceiver::PktReceiver(uint8 receiver_index, uint8 distributor_count)
	: receiver_index_(receiver_index)
	, distributor_count_(distributor_count)
	, bind_cpu_core_(false)
	, cpu_core_index_(0)
	, stop_flag_(false)
	, batch_proc_(false)
	, dropped_pkt_num_(0)
	, received_pkt_num_(0)
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktReceiver::Init()
{
	if (!ConstructPktProcessor())
	{
		LOG(ERROR) << "Fail to construct packet processor";
		return false;
	}

	if (!ConstructPktFifo())
	{
		LOG(ERROR) << "Fail to construct packet FIFO";
		return false;
	}

	GET_TMAS_CONFIG_BOOL("global.netmap.bind-process-thread-to-cpu", bind_cpu_core_);
	if (bind_cpu_core_)
	{
		CpuCoreManager& manager = CpuCoreManager::GetInstance();
		if (!manager.GetUnboundCpuCore(cpu_core_index_))
		{
			LOG(ERROR) << "Fail to bind receiver thead to cpu core";
			return false;
		}
	}

	bool debug = false;
	GET_TMAS_CONFIG_BOOL("global.debug.print-received-packet-number", debug);
	if (debug)
	{
		uint32 interval;
		GET_TMAS_CONFIG_INT("global.debug.print-interval", interval);
		if (interval == 0)
		{
			LOG(WARNING) << "debug.print-interval set to 1";
			interval = 1;
		}

		debug_timer_.reset(new FreeTimer(
			boost::bind(&PktReceiver::DebugProc, this), interval));
	}

	GET_TMAS_CONFIG_BOOL("global.netmap.batch-dispatch-packet", batch_proc_);

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ľ���
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktReceiver::Start()
{
	LOG(INFO) << "Start packet receiver " << (uint16)receiver_index_;

	proc_thread_.reset(new boost::thread(
		boost::bind(&PktReceiver::ThreadFunc, this)));

	if (bind_cpu_core_)
	{
		BindThreadToCpuCore(proc_thread_, cpu_core_index_);
	}

	if (debug_timer_)
	{
		debug_timer_->Start();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ֹͣ���Ľ���
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktReceiver::Stop()
{
	stop_flag_ = true;

	proc_thread_->join();
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ĵ�����
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktReceiver::ConstructPktProcessor()
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
 * ��  ��: �������ձ��Ļ�����
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktReceiver::ConstructPktFifo()
{
	// �������ļ���ȡ�����豸����
	uint32 fifo_size;
	GET_TMAS_CONFIG_INT("global.netmap.packet-process-fifo-size", fifo_size);

	if (fifo_size == 0 || fifo_size > 1024 * 1024 * 64)
	{
		LOG(ERROR) << "Invalid packet-process-fifo-size : " << fifo_size;
		return false;
	}

	TMAS_ASSERT(distributor_count_ > 0);

	for (uint8 i = 0; i < distributor_count_; i++)
	{
		pkt_fifos_.push_back(PktFifoSP(new CircularFifo<NetmapPktInfo>(fifo_size)));
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����һ���̣߳�ѭ���ӻ�������ȡ���Ĳ����д���
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktReceiver::ThreadFunc()
{
	if (batch_proc_)
	{
		BatchThreadProc();
	}
	else
	{
		OneByOneThreadProc();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������Ĵ���ģʽ
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktReceiver::OneByOneThreadProc()
{
	uint8 fifo_index = 0;
	uint8 try_times = 0;
	uint8 fifo_count = pkt_fifos_.size();
	NetmapPktInfo* fifo_pkt_info = nullptr;

	while (!stop_flag_)
	{
		// ����ѭ����ȡ���distributor�ı���
		for (try_times = 0; try_times < fifo_count; ++try_times)
		{
			fifo_pkt_info = pkt_fifos_[fifo_index]->GetReadableItem();
			if (fifo_pkt_info) break; // �б��Ŀɶ�

			fifo_index = (fifo_index + 1) % fifo_count;
		}

		// û�ж���������ȴ�һ��ʱ�������
		if (try_times >= fifo_count)
		{
			usleep(1);
			continue;
		}

		// �ߵ��⣬˵���Ѿ���ȡ���˱���

		PktInfo pkt_info;
		pkt_info.pkt.buf = fifo_pkt_info->data;
		pkt_info.pkt.len = fifo_pkt_info->len;
		pkt_info.arrive  = GetMicroSecond();

		pkt_processor_->ProcessMsg(MSG_PKT, (void*)(&pkt_info));

		// ֪ͨPktCapturer�������Ѿ�������
		fifo_pkt_info->pkt_capturer->PktProcessed(
			fifo_pkt_info->ring_index, fifo_pkt_info->slot_index);

		// ���Ķ�ȡ��ϣ�ˢ�µ�ǰ������
		pkt_fifos_[fifo_index]->FinishedRead();

		// ˢ���´ζ�ȡ���Ļ���������
		fifo_index = (fifo_index + 1) % fifo_count;

		received_pkt_num_++;
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ĵ���ģʽ
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktReceiver::BatchThreadProc()
{
	uint8 fifo_index = 0;
	uint8 try_times = 0;
	uint8 fifo_count = pkt_fifos_.size();
	std::vector<NetmapPktInfo*> pkt_container;

	while (!stop_flag_)
	{
		// ����ѭ����ȡ���distributor�ı���
		for (try_times = 0; try_times < fifo_count; ++try_times)
		{
			if (pkt_fifos_[fifo_index]->GetMultiReadableItems(pkt_container))
			{
				break; // �б��Ŀɶ�
			}

			fifo_index = (fifo_index + 1) % fifo_count;
		}

		// û�ж���������ȴ�һ��ʱ�������
		if (try_times >= fifo_count)
		{
			usleep(1);
			continue;
		}

		// �ߵ��⣬˵���Ѿ���ȡ���˱���

		for (NetmapPktInfo* fifo_pkt_info : pkt_container)
		{
			PktInfo pkt_info;
			pkt_info.pkt.buf = fifo_pkt_info->data;
			pkt_info.pkt.len = fifo_pkt_info->len;
			pkt_info.arrive  = GetMicroSecond();

			//pkt_processor_->ProcessMsg(MSG_PKT, (void*)(&pkt_info));

			// ֪ͨPktCapturer�������Ѿ�������
			fifo_pkt_info->pkt_capturer->PktProcessed(
				fifo_pkt_info->ring_index, fifo_pkt_info->slot_index);

			received_pkt_num_++;
		}

		// ���Ķ�ȡ��ϣ�ˢ�µ�ǰѭ��������
		pkt_fifos_[fifo_index]->FinishedReadMulti(pkt_container.size());
		pkt_container.clear();

		// ˢ���´ζ�ȡ���ĵ�ѭ��������
		fifo_index = (fifo_index + 1) % fifo_count;
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ӡ
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktReceiver::DebugProc()
{
	LOG(INFO) << "Packet receiver " << (uint16)receiver_index_ << " "
		      << "received packets " << received_pkt_num_ << " "
			  << "dropped packets " << dropped_pkt_num_;
}

}