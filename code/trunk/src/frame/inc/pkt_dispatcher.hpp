/*#############################################################################
 * 文件名   : pkt_dispatcher.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : PktCapturer类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_DISPATCHER
#define BROADINTER_PKT_DISPATCHER

#include <vector>
#include <queue>
#include <list>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

#include "tmas_typedef.hpp"
#include "frame_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 数据分发器
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
class PktDispatcher : public boost::noncopyable
{
public:
	PktDispatcher(uint8 index, PktReceiverVec& pkt_receivers);
	~PktDispatcher();

	bool Init();

	void DispatchPkt(const NetmapPktInfo& pkt_info);

	void DispatchPktBatch(NetmapPktInfo** pkts, uint32 size);
	
private:
	void DispatchPktRoundRobin(const NetmapPktInfo& pkt_info);
	void DispatchPktByIp(const NetmapPktInfo& pkt_info);
	void DispatchPktByIpAndPort(const NetmapPktInfo& pkt_info);

	void ConstructDispatchFuncs();
	void BatchDispatchInit();

	void DispatchPktRoundRobinBatch(NetmapPktInfo** pkts, uint32 size);
	void DispatchPktByIpBatch(NetmapPktInfo** pkts, uint32 size);
	void DispatchPktByIpAndPortBatch(NetmapPktInfo** pkts, uint32 size);

private:

	enum DispatchPolicy
	{
		DP_RR = 0,
		DP_IP,
		DP_IP_PORT,

		DP_BUTT
	};

	typedef void (PktDispatcher::*DispatchFunc)(const NetmapPktInfo&);
	typedef void (PktDispatcher::*BatchDispatchFunc)(NetmapPktInfo**, uint32);

	typedef std::vector<NetmapPktInfo*> BatchPktCacher;

private:
	// 所属PktDistributor的索引
	uint8 distributor_index_;

	// 每个dispatcher可能对应多个receiver
	PktReceiverVec& pkt_receivers_;

	// receiver的数量由配置确定
	uint8 pkt_receiver_count_;

	// 报文分发策略，可配置
	DispatchPolicy policy_;
	
	// 表驱动分发处理函数
	DispatchFunc dispatch_funcs_[DP_BUTT];

	// 批量分发报文缓存容器
	std::vector<BatchPktCacher> batch_pkt_cachers_;

	// 批量分发表驱动分发处理函数
	BatchDispatchFunc batch_dispatch_funcs_[DP_BUTT];
};

}

#endif