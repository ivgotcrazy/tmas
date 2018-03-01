/*#############################################################################
 * 文件名   : pkt_capturer.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月19日
 * 文件描述 : PktCapturer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: 计算循环缓冲区从start到end两个位置之间的空间大小。
 *         注意：如果两个位置重叠，则认为空间为0。
 * 参  数: [in] ring_size 循环缓冲区大小
 *         [in] start 起始位置
 *         [in] end 结束位置
 * 返回值: 空间大小
 * 修  改:
 *   时间 2014年04月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
static inline uint32 GetRingSlotCount(uint32 ring_size, uint32 start, uint32 end)
{
	return (end >= start) ? (end - start) : (ring_size - (start - end));	
}

/*-----------------------------------------------------------------------------
 * 描  述: 判断指定位置是否在循环缓冲区指定范围内
 * 参  数: [in] pos 指定位置
 *         [in] start 起始位置
 *         [in] end 结束位置
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年04月23日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 获取最大循环缓冲区容量
 * 参  数: 
 * 返回值: 最大循环缓冲区容量
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 启动抓包
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 停止抓包
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 获取过滤规则
 * 参  数: 
 * 返回值: 过滤规则
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
std::string PktCapturer::GetPktFilter() 
{ 
	std::string filter_str;
	GET_TMAS_CONFIG_STR("global.capture.capture-filter", filter_str);

	DLOG(INFO) << "Capture filter | " << filter_str;

	return filter_str;
}

/*-----------------------------------------------------------------------------
 * 描  述: 打开捕获网卡
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 创建于netmap环形缓冲区对应的结构，用于实现应用层的零拷贝分发
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月23日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 设置捕获规则
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool PktCapturer::SetCaptureFilter()
{
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文捕获线程处理函数
 * 参  数: [in] session 抓包网卡
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
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

		// 遍历所有循环缓冲区接收报文
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
 * 描  述: 接收ring中的所有报文
 * 参  数: [in] ring_id 环形缓冲区索引
 *         [in] ring 环形缓冲区
 *         [in] limit 环形缓冲区报文数
 * 返回值: 
 * 修  改:
 *   时间 2014年04月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktCapturer::ReceivePackets(int ring_id, struct netmap_ring *ring)
{

	unsigned int cur = ring->cur;
	unsigned int limit = nm_ring_space(ring);

	read_pkt_times_++;

	uint32& proc_index = netmap_rx_rings_[ring_id]->proc_index;
	uint32& read_index = netmap_rx_rings_[ring_id]->read_index;

	// 第一次读取报文需要初始化
	if (proc_index == 0xFFFFFFFF)
	{
		read_index = proc_index = cur;
	}

	uint32 pkt_pos = 0; // 报文插入容器的位置

	for (uint32 i = 0; i < limit; i++)
	{
		struct netmap_slot *slot = &ring->slot[cur];
		char *data = NETMAP_BUF(ring, slot->buf_idx);

		// 这些slot已经读取过，但还没来得及处理，不要再次读取
		if (IsPosInRingRange(cur, proc_index, read_index))
		{
			cur = nm_ring_next(ring, cur);
			continue;
		}

		// 标记此slot已分发但是未处理
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

		read_index = cur; // 刷新读取报文游标

		captured_pkt_num_++;
		read_pkt_num_++;
	}

	if (batch_dispatch_ && pkt_pos != 0)
	{
		pkt_dispatcher_->DispatchPktBatch(pkt_container_, pkt_pos);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 内核缓冲区的slot被分发出去后，不能立即释放，需要等待应用层处理完
 *         后才能够归还。
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktCapturer::RefreshNetmapRing(int ring_id)
{
	NetmapRingInfo* ring_info = netmap_rx_rings_[ring_id];

	// 可能未使用
	if (ring_info->proc_index == ring_info->read_index) return;

	struct netmap_ring *rxring = ring_info->ring;
	unsigned int cur = ring_info->proc_index;

	// 计算分发出去的slot数量
	uint32 read_num = GetRingSlotCount(ring_info->ring->num_slots, 
		ring_info->proc_index, ring_info->read_index);

	// 看看是否有被分发出去的slot已经处理完毕
	for (uint32 j = 0; j < read_num; j++)
	{
		if (ring_info->ring_slots[cur]->load()) break;
		cur = nm_ring_next(rxring, cur);
	}

	// 刷新内核环形缓冲区，将处理完毕的slot空间释放
	if (cur != ring_info->proc_index)
	{
		ring_info->proc_index = cur;
		rxring->head = rxring->cur = cur;
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 打印捕获到报文数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月23日
 *   作者 teck_zhou
 *   描述 创建
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