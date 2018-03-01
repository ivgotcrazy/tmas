/*#############################################################################
 * 文件名   : eth_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月21日
 * 文件描述 : EthMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_ETH_MONITOR
#define BROADINTER_ETH_MONITOR

#include "pkt_processor.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: ETH层数据监测与分析
 * 作  者: teck_zhou
 * 时  间: 2014年01月21日
 ******************************************************************************/
template<class Next, class Succ>
class EthMonitor : public PktProcessor<EthMonitor<Next, Succ>, Next, Succ>
{
public:
	bool Init();

	//--- TODO: 待优化访问限制

	// 报文消息处理
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

private:

	// 打印以太层信息
	inline void PrintEthInfo(const PktMsgSP& pkt_msg);
};

}

#include "eth_monitor-inl.hpp"

#endif
