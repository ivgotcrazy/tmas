/*#############################################################################
 * �ļ���   : pkt_capturer.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��19��
 * �ļ����� : PktCapturer��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <boost/bind.hpp>

#include "pkt_capturer.hpp"
#include "tmas_util.hpp"
#include "mem_buf_pool.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_cfg.hpp"
#include "pkt_dispatcher.hpp"
#include "cpu_core_manager.hpp"
#include "pkt_resolver.hpp"


namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ����ѭ����������start��end����λ��֮��Ŀռ��С��
 *         ע�⣺�������λ���ص�������Ϊ�ռ�Ϊ0��
 * ��  ��: [in] ring_size ѭ����������С
 *         [in] start ��ʼλ��
 *         [in] end ����λ��
 * ����ֵ: �ռ��С
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
static inline uint32 GetRingSlotCount(uint32 ring_size, uint32 start, uint32 end)
{
	return (end >= start) ? (end - start) : (ring_size - (start - end));	
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ж�ָ��λ���Ƿ���ѭ��������ָ����Χ��
 * ��  ��: [in] pos ָ��λ��
 *         [in] start ��ʼλ��
 *         [in] end ����λ��
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
static inline bool IsPosInRingRange(uint32 pos, uint32 start, uint32 end)
{
	if (end == start) return false;

	if (end > start) 
		return (pos >= start && pos < end);
	else 
		return (pos >= start || pos < end);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktCapturer::PktCapturer(const string& device_name, const PktDispatcherSP& dispacher) 
	: device_name_(device_name)
	, capture_nmd_(nullptr)
	, pkt_dispatcher_(dispacher)
	, bind_cpu_core_(false)
	, cpu_core_index_(0)
	, stop_flag_(false)
	, batch_dispatch_(false)
	, pkt_container_(nullptr)
	, captured_pkt_num_(0)
	, read_pkt_num_(0)
	, read_pkt_times_(0)
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktCapturer::~PktCapturer()
{
	for (NetmapRingInfo* ring_info : netmap_rx_rings_)
	{
		for (std::atomic<bool>* slot : ring_info->ring_slots)
		{
			delete slot;
		}
		delete ring_info;
	}

	if (!stop_flag_) Stop();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktCapturer::Init()
{
	if (!OpenCaptureDevice())
	{
		LOG(ERROR) << "Fail to open capture interface";
		return false;
	}

	ConstructNetmapRingInfo();

	if (!SetCaptureFilter())
	{
		LOG(ERROR) << "Fail to set capture filter";
		return false;
	}

	GET_TMAS_CONFIG_BOOL("global.capture.bind-capture-thread-to-cpu", bind_cpu_core_);
	if (bind_cpu_core_)
	{
		CpuCoreManager& manager = CpuCoreManager::GetInstance();
		if (!manager.GetUnboundCpuCore(cpu_core_index_))
		{
			LOG(ERROR) << "Fail to bind capture thead to cpu core";
			return false;
		}
	}

	bool print = false;
	GET_TMAS_CONFIG_BOOL("global.debug.print-captured-packet-number", print);
	if (print)
	{
		uint32 interval;
		GET_TMAS_CONFIG_INT("global.debug.print-interval", interval);
		if (interval == 0)
		{
			LOG(WARNING) << "debug.print-interval set to 1";
			interval = 1;
		}

		debug_timer_.reset(new FreeTimer(
			boost::bind(&PktCapturer::DebugProc, this), interval));
	}

	GET_TMAS_CONFIG_BOOL("global.netmap.batch-dispatch-packet", batch_dispatch_);
	if (batch_dispatch_)
	{
		uint32 container_size = GetNetmapMaxRingSize();
		pkt_container_ = (NetmapPktInfo**)(new char[sizeof(NetmapPktInfo*) * container_size]);
		for (uint32 i = 0; i < container_size; i++)
		{
			pkt_container_[i] = new NetmapPktInfo;
		}
	}

	LOG(INFO) << "Initialize netmap packet capturer successfully";

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ���ѭ������������
 * ��  ��: 
 * ����ֵ: ���ѭ������������
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
uint32 PktCapturer::GetNetmapMaxRingSize()
{
	uint32 max_ring_size = 0;

	for (int i = capture_nmd_->first_rx_ring; i <= capture_nmd_->last_rx_ring; ++i)
	{
		struct netmap_ring *ring = NETMAP_RXRING(capture_nmd_->nifp, i);

		if (ring->num_slots > max_ring_size)
		{
			max_ring_size = ring->num_slots;
		}
	}

	return max_ring_size;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����ץ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::Start()
{
	LOG(INFO) << "Start to capture packets on " << device_name_;

	capture_thread_.reset(new boost::thread(
		boost::bind(&PktCapturer::ThreadFunc, this)));

	if (bind_cpu_core_)
	{
		BindThreadToCpuCore(capture_thread_, cpu_core_index_);
	}

	if (debug_timer_)
	{
		debug_timer_->Start();
	}	
}

/*-----------------------------------------------------------------------------
 * ��  ��: ֹͣץ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::Stop()
{
	stop_flag_ = true;

	if (capture_thread_)
	{
		capture_thread_->join();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ���˹���
 * ��  ��: 
 * ����ֵ: ���˹���
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
std::string PktCapturer::GetPktFilter() 
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
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktCapturer::OpenCaptureDevice()
{
	std::string netmap_device = "netmap:" + device_name_;

	uint64_t nmd_flags = 0;
	nmd_flags |= (NM_OPEN_IFNAME);

	struct nm_desc base_nmd;
	bzero(&base_nmd, sizeof(base_nmd));

	base_nmd.req.nr_ringid |= NETMAP_NO_TX_POLL;

	capture_nmd_ = nm_open(netmap_device.c_str(), NULL, nmd_flags, &base_nmd);
	if (NULL == capture_nmd_)
	{
		LOG(ERROR) << "Unable to open " << device_name_
			       << " | " << strerror(errno);
		return false;
	}

	LOG(INFO) << "Open netmap device successfully | rx-ring number: " 
		      << capture_nmd_->last_rx_ring + 1; 

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������netmap���λ�������Ӧ�Ľṹ������ʵ��Ӧ�ò���㿽���ַ�
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::ConstructNetmapRingInfo()
{
	for (int i = capture_nmd_->first_rx_ring; i <= capture_nmd_->last_rx_ring; ++i)
	{
		NetmapRingInfo *ring_info = new NetmapRingInfo;

		ring_info->ring = NETMAP_RXRING(capture_nmd_->nifp, i);
		
		for (uint32 j = 0; j < ring_info->ring->num_slots; j++)
		{
			ring_info->ring_slots.push_back(new std::atomic<bool>(false));
		}

		netmap_rx_rings_.push_back(ring_info);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ò������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktCapturer::SetCaptureFilter()
{
	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĳ����̴߳�����
 * ��  ��: [in] session ץ������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::ThreadFunc()
{
	netmap_if *nifp = capture_nmd_->nifp;
	pollfd pfd = { .fd = capture_nmd_->fd, .events = POLLIN };

	while (!stop_flag_)
	{
		int ret = poll(&pfd, 1, 1 * 1000);
		if (ret < 0)
		{
			LOG(ERROR) << "Poll error | " << strerror(errno);
			return;
		}

		if (pfd.revents & POLLERR)
		{
			LOG(ERROR) << "Poll error";
			return;
		}

		// ��������ѭ�����������ձ���
		for (int ring_index = capture_nmd_->first_rx_ring; 
			 ring_index <= capture_nmd_->last_rx_ring; ring_index++)
		{
			struct netmap_ring *rxring = NETMAP_RXRING(nifp, ring_index);

			if (nm_ring_empty(rxring)) continue;

			ReceivePackets(ring_index, rxring);

			RefreshNetmapRing(ring_index);
		}
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����ring�е����б���
 * ��  ��: [in] ring_id ���λ���������
 *         [in] ring ���λ�����
 *         [in] limit ���λ�����������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::ReceivePackets(int ring_id, struct netmap_ring *ring)
{

	unsigned int cur = ring->cur;
	unsigned int limit = nm_ring_space(ring);

	read_pkt_times_++;

	uint32& proc_index = netmap_rx_rings_[ring_id]->proc_index;
	uint32& read_index = netmap_rx_rings_[ring_id]->read_index;

	// ��һ�ζ�ȡ������Ҫ��ʼ��
	if (proc_index == 0xFFFFFFFF)
	{
		read_index = proc_index = cur;
	}

	uint32 pkt_pos = 0; // ���Ĳ���������λ��

	for (uint32 i = 0; i < limit; i++)
	{
		struct netmap_slot *slot = &ring->slot[cur];
		char *data = NETMAP_BUF(ring, slot->buf_idx);

		// ��Щslot�Ѿ���ȡ��������û���ü�������Ҫ�ٴζ�ȡ
		if (IsPosInRingRange(cur, proc_index, read_index))
		{
			cur = nm_ring_next(ring, cur);
			continue;
		}

		// ��Ǵ�slot�ѷַ�����δ����
		netmap_rx_rings_[ring_id]->ring_slots[cur]->store(true);

		if (batch_dispatch_)
		{
			pkt_container_[pkt_pos]->data = data;
			pkt_container_[pkt_pos]->len  = slot->len;
			pkt_container_[pkt_pos]->slot_index = cur;
			pkt_container_[pkt_pos]->ring_index = ring_id;
			pkt_container_[pkt_pos]->pkt_capturer = this;

			pkt_pos++;
		}
		else
		{
			NetmapPktInfo nm_pkt_info(data, slot->len, cur, ring_id, this);
			pkt_dispatcher_->DispatchPkt(nm_pkt_info);
		}

		cur = nm_ring_next(ring, cur);

		read_index = cur; // ˢ�¶�ȡ�����α�

		captured_pkt_num_++;
		read_pkt_num_++;
	}

	if (batch_dispatch_ && pkt_pos != 0)
	{
		pkt_dispatcher_->DispatchPktBatch(pkt_container_, pkt_pos);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ں˻�������slot���ַ���ȥ�󣬲��������ͷţ���Ҫ�ȴ�Ӧ�ò㴦����
 *         ����ܹ��黹��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::RefreshNetmapRing(int ring_id)
{
	NetmapRingInfo* ring_info = netmap_rx_rings_[ring_id];

	// ����δʹ��
	if (ring_info->proc_index == ring_info->read_index) return;

	struct netmap_ring *rxring = ring_info->ring;
	unsigned int cur = ring_info->proc_index;

	// ����ַ���ȥ��slot����
	uint32 read_num = GetRingSlotCount(ring_info->ring->num_slots, 
		ring_info->proc_index, ring_info->read_index);

	// �����Ƿ��б��ַ���ȥ��slot�Ѿ��������
	for (uint32 j = 0; j < read_num; j++)
	{
		if (ring_info->ring_slots[cur]->load()) break;
		cur = nm_ring_next(rxring, cur);
	}

	// ˢ���ں˻��λ���������������ϵ�slot�ռ��ͷ�
	if (cur != ring_info->proc_index)
	{
		ring_info->proc_index = cur;
		rxring->head = rxring->cur = cur;
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ӡ���񵽱�����
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktCapturer::DebugProc()
{
	uint32 average = 0;

	if (read_pkt_times_ == 0)
	{
		average = 0;
	}
	else
	{
		average = read_pkt_num_ / read_pkt_times_;
	}

	LOG(INFO) << "Device " << device_name_ << " captured " 
		      << captured_pkt_num_ << " packets | " 
			  << average;

	read_pkt_num_ = read_pkt_times_ = 0;
}

}