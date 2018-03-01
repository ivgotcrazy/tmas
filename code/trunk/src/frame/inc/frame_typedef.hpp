/*#############################################################################
 * 文件名   : frame_typedef.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月22日
 * 文件描述 : frame相关类型定义
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_FRAME_TYPEDEF
#define BROADINTER_FRAME_TYPEDEF

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>

#include <atomic>
#include <vector>
#include "tmas_typedef.hpp"

namespace BroadInter
{

struct NetmapRingInfo
{
	NetmapRingInfo() : read_index(0xFFFFFFFF), proc_index(0xFFFFFFFF), ring(0) {}

	// netmap内核环形缓冲区映像，用于控制内核报文接收
	// true: 报文已经分发出去，但是还未处理
	// false: 报文未分发或者已经处理
	std::vector<std::atomic<bool>*> ring_slots;

	// 已经分发出去的slot索引
	uint32 read_index;

	// 已经处理完成的slot索引
	uint32 proc_index;

	// 对应的内核环形缓冲区
	struct netmap_ring *ring;
};

struct NetmapPktInfo
{
	NetmapPktInfo() : data(0), len(0), slot_index(0)
		, ring_index(0), pkt_capturer(0) {}

	NetmapPktInfo(char* buf, uint32 size, uint32 slot, 
		uint32 ring, PktCapturer* p)
		: data(buf), len(size), slot_index(slot)
		, ring_index(ring), pkt_capturer(p) {}

	// 报文数据
	char* data;
	uint32 len;

	// 报文所在环形缓冲区的slot索引
	uint32 slot_index;

	// slot所属环形缓冲区
	uint32 ring_index;

	// 所属报文捕获器
	PktCapturer* pkt_capturer;
};

}

#endif