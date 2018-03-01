/*#############################################################################
 * 文件名   : pkt_distributor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月19日
 * 文件描述 : PktDistributor类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "pkt_distributor.hpp"
#include "pkt_dispatcher.hpp"
#include "pkt_capturer.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] index 全局分配的分发器索引
 *         [in] device 分发器对应的抓包网卡
 *         [in] receivers 报文接收器
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
PktDistributor::PktDistributor(uint8 index, const string& device, 
	PktReceiverVec& receivers)
	: distributor_index_(index)
	, device_name_(device)
	, pkt_receivers_(receivers)
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool PktDistributor::Init()
{
	pkt_dispatcher_.reset(new PktDispatcher(distributor_index_, pkt_receivers_));
	TMAS_ASSERT(pkt_dispatcher_);
	
	if (!pkt_dispatcher_->Init())
	{
		LOG(ERROR) << "Fail to initialize packet dispatcher";
		return false;
	}

	pkt_capturer_.reset(new PktCapturer(device_name_, pkt_dispatcher_));
	TMAS_ASSERT(pkt_dispatcher_);

	if (!pkt_capturer_->Init())
	{
		LOG(ERROR) << "Fail to initialize packet capturer " << device_name_;
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 启动报文分发器，开始从网卡抓取报文并进行分发
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDistributor::Start()
{
	LOG(INFO) << "Start packet distributor " << (uint16)distributor_index_;

	pkt_capturer_->Start();
}

/*-----------------------------------------------------------------------------
 * 描  述: 停止报文分发器
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void PktDistributor::Stop()
{
	pkt_capturer_->Stop();
}

}