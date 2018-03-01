/*#############################################################################
 * 文件名   : pkt_receiver.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月19日
 * 文件描述 : PktReceiver类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_RECEIVER
#define BROADINTER_PKT_RECEIVER

#include <atomic>
#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "frame_typedef.hpp"
#include "circular_fifo.hpp"
#include "timer.hpp"
#include "tmas_assert.hpp"
#include "pkt_capturer.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 负责报文的捕获和再分发
 * 作  者: teck_zhou
 * 时  间: 2014年04月19日
 ******************************************************************************/
class PktReceiver : public boost::noncopyable
{
public:
	PktReceiver(uint8 receiver_index, uint8 distributor_count);

	bool Init();

	void Start();
	void Stop();

	inline void ReceivePkt(uint8 index, const NetmapPktInfo& pkt_info);
	
	inline void ReceivePktBatch(uint8 index, const std::vector<NetmapPktInfo*>& pkts);

private:
	// 构造报文处理链
	bool ConstructPktProcessor();

	// 构造环形缓冲区
	bool ConstructPktFifo();

	// 报文处理线程
	void ThreadFunc();

	void OneByOneThreadProc();

	void BatchThreadProc();

	void DebugProc();

private:
	uint8 receiver_index_;

	// 根据分发器数量来创建报文缓冲区
	uint8 distributor_count_;
	
	// 报文处理器
	EthMonitorTypeSP pkt_processor_;

	// 报文缓冲区，每个网卡分配一个缓冲队列
	typedef boost::shared_ptr<CircularFifo<NetmapPktInfo> > PktFifoSP;
	std::vector<PktFifoSP> pkt_fifos_;

	// 报文处理线程
	boost::shared_ptr<boost::thread> proc_thread_;

	// 将线程绑定到CPU核心
	bool bind_cpu_core_;
	uint8 cpu_core_index_;

	// 线程和定时器退出标志
	bool stop_flag_;

	// 是否启用批量处理模式
	bool batch_proc_;

	// debug相关
	boost::scoped_ptr<FreeTimer> debug_timer_;
	uint64 dropped_pkt_num_;
	uint64 received_pkt_num_;
};

/*-----------------------------------------------------------------------------
 * 描  述: 接收报文，将捕获报文拷贝到环形缓冲区
           在多网卡抓包场景下，会有多个线程访问此函数，不过由于我们已经为每个
		   网卡都分配了一个环形缓冲区，因此，每个线程访问的环形缓冲区是独立的，
		   无需加锁；同时，各个线程对缓冲区容器只读，因此，对缓冲区容器的访问
		   也不需要加锁。
 * 参  数: [in] index 分发器索引(对应一个抓包网卡)
 *         [in] pkt_info 报文信息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
inline void PktReceiver::ReceivePkt(uint8 index, const NetmapPktInfo& pkt_info)
{
	TMAS_ASSERT(index < pkt_fifos_.size());

	NetmapPktInfo* fifo_pkt_info = pkt_fifos_[index]->GetWritableItem();
	if (!fifo_pkt_info)
	{
		dropped_pkt_num_++;
		pkt_info.pkt_capturer->PktProcessed(
			pkt_info.ring_index, pkt_info.slot_index);
		return;
	}

	std::memcpy(fifo_pkt_info, &pkt_info, sizeof(NetmapPktInfo));

	pkt_fifos_[index]->FinishedWrite();
}

/*-----------------------------------------------------------------------------
 * 描  述: 批量接收报文
 * 参  数: [in] index 分发器索引(对应一个抓包网卡)
 *         [in] pkts 报文指针数组
 *         [in] size 报文个数
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
inline void PktReceiver::ReceivePktBatch(uint8 index, const std::vector<NetmapPktInfo*>& pkts)
{
	TMAS_ASSERT(index < pkt_fifos_.size());

	uint32 pkt_num = pkts.size();

	std::vector<NetmapPktInfo*> pkt_infos;
	if (!pkt_fifos_[index]->GetMultiWritableItems(pkt_num, pkt_infos))
	{
		dropped_pkt_num_++;

		for (NetmapPktInfo* pkt_info : pkts)
		{
			pkt_info->pkt_capturer->PktProcessed(
				pkt_info->ring_index, pkt_info->slot_index);
		}

		return;
	}

	for (uint32 i = 0; i < pkt_num; i++)
	{
		std::memcpy(pkt_infos[i], pkts[i], sizeof(NetmapPktInfo));
	}

	pkt_fifos_[index]->FinishedWriteMulti(pkt_num);
}

}

#endif