/*#############################################################################
 * 文件名   : pkt_capturer.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2013年12月31日
 * 文件描述 : PktCapturer类声明
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_CAPTURER
#define BROADINTER_PKT_CAPTURER

//-------------- netmap ----------------
#include <stdio.h>
#include <sys/poll.h>
//--------------------------------------

#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <glog/logging.h>

#include "tmas_typedef.hpp"
#include "frame_typedef.hpp"
#include "tmas_assert.hpp"
#include "message.hpp"
#include "timer.hpp"

namespace BroadInter
{

using std::string;

/******************************************************************************
 * 描述：报文捕获器
 * 作者：teck_zhou
 * 时间：2014年04月19日
 *****************************************************************************/
class PktCapturer : public boost::noncopyable
{
public:
	PktCapturer(const string& device_name, const PktDispatcherSP& dispacher);
	~PktCapturer();

	bool Init();

	void Start();
	void Stop();

	inline void PktProcessed(uint32 ring_index, uint32 slot_index);

private:
	// 获取报文过滤规则
	string GetPktFilter();

	// 报文捕获线程
	void ThreadFunc();

	// 打开报文捕获网卡
	bool OpenCaptureDevice();

	// 构造netmap环形缓冲区
	void ConstructNetmapRingInfo();

	bool SetCaptureFilter();

	void ReceivePackets(int ring_id, struct netmap_ring *ring);

	void RefreshNetmapRing(int ring_id);

	void DebugProc();

	uint32 GetNetmapMaxRingSize();

private:
	// 抓包网卡名称
	string device_name_;

	// netmap用户态句柄
	struct nm_desc* capture_nmd_;

	// 报文捕获后，直接送给PktDispacher
	PktDispatcherSP pkt_dispatcher_;

	// 每个网卡使用一个线程抓包
	boost::shared_ptr<boost::thread> capture_thread_;

	// 是否将报文捕获线程绑定到CPU核心
	bool bind_cpu_core_;

	// 报文捕获线程绑定到的CPU核心索引
	uint8 cpu_core_index_;

	// 线程和定时器退出标志
	bool stop_flag_;

	// 对应netmap内核环形缓冲区
	std::vector<NetmapRingInfo*> netmap_rx_rings_;

	// 支持先全部从内核捕获上来，再一次性分发
	bool batch_dispatch_;
	NetmapPktInfo** pkt_container_;

	// Debug
	boost::scoped_ptr<FreeTimer> debug_timer_;
	uint64 captured_pkt_num_;	// 总共捕获报文数
	uint64 read_pkt_num_;		// 每次打印后清零
	uint64 read_pkt_times_;		// 从ring中读了多少次
};

/*-----------------------------------------------------------------------------
 * 描  述: 接收器处理完报文后，需要刷新slot状态
 * 参  数: [in] ring_index 循环缓冲区索引
 *         [in] slot_index slot索引
 * 返回值:
 * 修  改:
 *   时间 2014年04月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
inline void PktCapturer::PktProcessed(uint32 ring_index, uint32 slot_index)
{
	TMAS_ASSERT(ring_index < netmap_rx_rings_.size());
	TMAS_ASSERT(slot_index < netmap_rx_rings_[0]->ring_slots.size());

	netmap_rx_rings_[ring_index]->ring_slots[slot_index]->store(false);
}

}

#endif