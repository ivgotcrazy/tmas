/*#############################################################################
 * �ļ���   : frame_typedef.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��22��
 * �ļ����� : frame������Ͷ���
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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

	// netmap�ں˻��λ�����ӳ�����ڿ����ں˱��Ľ���
	// true: �����Ѿ��ַ���ȥ�����ǻ�δ����
	// false: ����δ�ַ������Ѿ�����
	std::vector<std::atomic<bool>*> ring_slots;

	// �Ѿ��ַ���ȥ��slot����
	uint32 read_index;

	// �Ѿ�������ɵ�slot����
	uint32 proc_index;

	// ��Ӧ���ں˻��λ�����
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

	// ��������
	char* data;
	uint32 len;

	// �������ڻ��λ�������slot����
	uint32 slot_index;

	// slot�������λ�����
	uint32 ring_index;

	// �������Ĳ�����
	PktCapturer* pkt_capturer;
};

}

#endif