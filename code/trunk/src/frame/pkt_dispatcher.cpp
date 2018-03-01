/*#############################################################################
 * 文件名   : pkt_dispatcher.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月20日
 * 文件描述 : PktCapturer类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <algorithm>
#include <glog/logging.h>

#include "pkt_dispatcher.hpp"
#include "pkt_capturer.hpp"
#include "mem_buf_pool.hpp"	
#include "pkt_processor.hpp"
#include "tmas_util.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_cfg.hpp"
#include "pkt_resolver.hpp"
#include "pkt_receiver.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] index 所属PktDistributor索引
 *         [in] pkt_receivers 报文分发接收对象
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktDispatcher::PktDispatcher(uint8 index, PktReceiverVec& pkt_receivers) 
	: distributor_index_(index)
	, pkt_receivers_(pkt_receivers)
	, pkt_receiver_count_(pkt_receivers_.size())
	, policy_(DP_IP)
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktDispatcher::~PktDispatcher()
{

}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool PktDispatcher::Init()
{
	// 从配置文件读取分发策略

	std::string policy;
	GET_TMAS_CONFIG_STR("global.netmap.packet-dispatch-policy", policy);

	if (policy == "rr" || policy == "round-robin")
	{
		LOG(INFO) << "Packet dispatch policy : rr(round-robin)";
		policy_ = DP_RR;
	}
	else if (policy == "ip")
	{
		LOG(INFO) << "Packet dispatch policy : ip";
		policy_ = DP_IP;
	}
	else if (policy == "ip-port")
	{
		LOG(INFO) << "Packet dispatch policy : ip-port";
		policy = DP_IP_PORT;
	}
	else
	{
		LOG(ERROR) << "Invalid packet-dispatch-policy : " << policy;
		return false;
	}

	// 保险起见，还是校验下
	if (pkt_receiver_count_ == 0)
	{
		LOG(ERROR) << "Empty packet receviers";
		return false;
	}

	ConstructDispatchFuncs();

	BatchDispatchInit();

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 构建分发函数表
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::ConstructDispatchFuncs()
{
	dispatch_funcs_[DP_RR]		= &PktDispatcher::DispatchPktRoundRobin;
	dispatch_funcs_[DP_IP]		= &PktDispatcher::DispatchPktByIp;
	dispatch_funcs_[DP_IP_PORT] = &PktDispatcher::DispatchPktByIpAndPort;
}

/*-----------------------------------------------------------------------------
 * 描  述: 批量分发相关初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::BatchDispatchInit()
{
	batch_dispatch_funcs_[DP_RR]	  = &PktDispatcher::DispatchPktRoundRobinBatch;
	batch_dispatch_funcs_[DP_IP]	  = &PktDispatcher::DispatchPktByIpBatch;
	batch_dispatch_funcs_[DP_IP_PORT] = &PktDispatcher::DispatchPktByIpAndPortBatch;

	batch_pkt_cachers_.resize(pkt_receiver_count_);
}

/*-----------------------------------------------------------------------------
 * 描  述: 批量分发
 * 参  数: [in] pkts 报文
 *         [in] size 报文数量
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktBatch(NetmapPktInfo** pkts, uint32 size)
{
	TMAS_ASSERT(policy_ < DP_BUTT);

	(this->*batch_dispatch_funcs_[policy_])(pkts, size);
}

/*-----------------------------------------------------------------------------
 * 描  述: 实现报文分发
 * 参  数: [in] pkt_info 报文信息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPkt(const NetmapPktInfo& pkt_info)
{
	TMAS_ASSERT(policy_ < DP_BUTT);

	(this->*dispatch_funcs_[policy_])(pkt_info);
}

/*-----------------------------------------------------------------------------
 * 描  述: 基于IP地址分发报文
 * 参  数: [in] pkt_info 报文信息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIp(const NetmapPktInfo& pkt_info)
{
	uint32 src_ip = PR::GetSourceIpN(pkt_info.data + 14);
	uint32 dst_ip = PR::GetDestIpN(pkt_info.data + 14);

	uint8 index = (src_ip + dst_ip) % pkt_receiver_count_;

	pkt_receivers_[index]->ReceivePkt(distributor_index_, pkt_info);
}

/*-----------------------------------------------------------------------------
 * 描  述: 轮询所有的报文接收器进行分发
 * 参  数: [in] pkt_info 报文信息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktRoundRobin(const NetmapPktInfo& pkt_info)
{
	static uint8 current_receiver = 0;

	pkt_receivers_[current_receiver]->ReceivePkt(distributor_index_, pkt_info);

	current_receiver = (current_receiver + 1) % pkt_receiver_count_;
}

/*-----------------------------------------------------------------------------
 * 描  述: 基于IP和Port分发报文
 * 参  数: [in] pkt_info 报文信息
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIpAndPort(const NetmapPktInfo& pkt_info)
{

}

/*-----------------------------------------------------------------------------
 * 描  述: 轮询所有的报文接收器进行批量分发
 * 参  数: [in] pkts 报文
 *         [in] size 报文数量
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktRoundRobinBatch(NetmapPktInfo** pkts, uint32 size)
{

}

/*-----------------------------------------------------------------------------
 * 描  述: 基于IP地址的批量分发
 * 参  数: [in] pkts 报文
 *         [in] size 报文数量
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIpBatch(NetmapPktInfo** pkts, uint32 size)
{
	// 先按照分发策略进行预分发，将报文打包在一起
	for (uint32 i = 0; i < size; i++)
	{
		uint32 src_ip = PR::GetSourceIpN(pkts[i]->data + 14);
		uint32 dst_ip = PR::GetDestIpN(pkts[i]->data + 14);

		uint8 receiver_index = (src_ip + dst_ip) % pkt_receiver_count_;

		batch_pkt_cachers_[receiver_index].push_back(pkts[i]);
	}

	// 将打包好的报文一次性分发给receiver
	for (uint8 i = 0; i < pkt_receiver_count_; i++)
	{
		// 有可能某一个receiver上没有分发到报文
		if (batch_pkt_cachers_[i].empty()) continue;

		pkt_receivers_[i]->ReceivePktBatch(distributor_index_, batch_pkt_cachers_[i]);

		batch_pkt_cachers_[i].clear();
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 基于IP和Port的批量分发
 * 参  数: [in] pkts 报文
 *         [in] size 报文数量
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDispatcher::DispatchPktByIpAndPortBatch(NetmapPktInfo** pkts, uint32 size)
{

}

}