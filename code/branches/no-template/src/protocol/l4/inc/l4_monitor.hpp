/*#############################################################################
 * 文件名   : l4_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : L4Monitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_L4_MONITOR
#define BROADINTER_L4_MONITOR

#include <boost/noncopyable.hpp>

#include "pkt_processor.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 传输层数据监听
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
class L4Monitor : public PktProcessor
{
public:
	bool Init();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;
	
private:
	bool ResolveL4Info(PktMsgSP& pkt_msg);
	uint8 GetPktDirection(const PktMsgSP& pkt_msg);

private:
	PktProcessorSP pkt_processor_;
};

}

#endif