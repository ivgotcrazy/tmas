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
class EthMonitor : public PktProcessor
{
public:
	bool Init();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	void PrintEthInfo(const PktMsgSP& pkt_msg);
};

}

#endif
