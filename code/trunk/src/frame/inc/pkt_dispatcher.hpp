/*#############################################################################
 * �ļ���   : pkt_dispatcher.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : PktCapturer������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ���ݷַ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
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
	// ����PktDistributor������
	uint8 distributor_index_;

	// ÿ��dispatcher���ܶ�Ӧ���receiver
	PktReceiverVec& pkt_receivers_;

	// receiver������������ȷ��
	uint8 pkt_receiver_count_;

	// ���ķַ����ԣ�������
	DispatchPolicy policy_;
	
	// �������ַ�������
	DispatchFunc dispatch_funcs_[DP_BUTT];

	// �����ַ����Ļ�������
	std::vector<BatchPktCacher> batch_pkt_cachers_;

	// �����ַ��������ַ�������
	BatchDispatchFunc batch_dispatch_funcs_[DP_BUTT];
};

}

#endif