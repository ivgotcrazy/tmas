/*#############################################################################
 * 文件名   : pkt_distributor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月19日
 * 文件描述 : PktDistributor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_DISTRIBUTOR
#define BROADINTER_PKT_DISTRIBUTOR

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * 描  述: 负责报文的捕获和再分发
 * 作  者: teck_zhou
 * 时  间: 2014年04月19日
 ******************************************************************************/
class PktDistributor : public boost::noncopyable
{
public:
	PktDistributor(uint8 index, 
		           const string& device, 
				   PktReceiverVec& receivers);

	bool Init();

	void Start();
	void Stop();

private:
	// 全局分配的索引
	uint8 distributor_index_;

	// 抓包网卡
	std::string device_name_;

	// 报文捕获器
	PktCapturerSP pkt_capturer_;

	// 报文分发器
	PktDispatcherSP pkt_dispatcher_;

	// 创建PktDispatcher使用
	PktReceiverVec& pkt_receivers_;
};

}

#endif