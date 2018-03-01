/*#############################################################################
 * �ļ���   : pkt_capturer.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2013��12��31��
 * �ļ����� : PktCapturer������
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
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
 * ���������Ĳ�����
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��04��19��
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
	// ��ȡ���Ĺ��˹���
	string GetPktFilter();

	// ���Ĳ����߳�
	void ThreadFunc();

	// �򿪱��Ĳ�������
	bool OpenCaptureDevice();

	// ����netmap���λ�����
	void ConstructNetmapRingInfo();

	bool SetCaptureFilter();

	void ReceivePackets(int ring_id, struct netmap_ring *ring);

	void RefreshNetmapRing(int ring_id);

	void DebugProc();

	uint32 GetNetmapMaxRingSize();

private:
	// ץ����������
	string device_name_;

	// netmap�û�̬���
	struct nm_desc* capture_nmd_;

	// ���Ĳ����ֱ���͸�PktDispacher
	PktDispatcherSP pkt_dispatcher_;

	// ÿ������ʹ��һ���߳�ץ��
	boost::shared_ptr<boost::thread> capture_thread_;

	// �Ƿ񽫱��Ĳ����̰߳󶨵�CPU����
	bool bind_cpu_core_;

	// ���Ĳ����̰߳󶨵���CPU��������
	uint8 cpu_core_index_;

	// �̺߳Ͷ�ʱ���˳���־
	bool stop_flag_;

	// ��Ӧnetmap�ں˻��λ�����
	std::vector<NetmapRingInfo*> netmap_rx_rings_;

	// ֧����ȫ�����ں˲�����������һ���Էַ�
	bool batch_dispatch_;
	NetmapPktInfo** pkt_container_;

	// Debug
	boost::scoped_ptr<FreeTimer> debug_timer_;
	uint64 captured_pkt_num_;	// �ܹ���������
	uint64 read_pkt_num_;		// ÿ�δ�ӡ������
	uint64 read_pkt_times_;		// ��ring�ж��˶��ٴ�
};

/*-----------------------------------------------------------------------------
 * ��  ��: �����������걨�ĺ���Ҫˢ��slot״̬
 * ��  ��: [in] ring_index ѭ������������
 *         [in] slot_index slot����
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��04��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
inline void PktCapturer::PktProcessed(uint32 ring_index, uint32 slot_index)
{
	TMAS_ASSERT(ring_index < netmap_rx_rings_.size());
	TMAS_ASSERT(slot_index < netmap_rx_rings_[0]->ring_slots.size());

	netmap_rx_rings_[ring_index]->ring_slots[slot_index]->store(false);
}

}

#endif