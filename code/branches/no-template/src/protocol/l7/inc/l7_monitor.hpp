/*#############################################################################
 * 文件名   : l7_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : L7Monitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_L7_MONITOR
#define BROADINTER_L7_MONITOR

#include <boost/noncopyable.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 应用层数据监听
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
class L7Monitor : public PktProcessor
{
public:
	bool Init();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	PktProcessorSP pkt_processor_;
};

}

#endif