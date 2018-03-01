/*#############################################################################
 * 文件名   : pkt_processor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : PktProcessor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tmas_assert.hpp"
#include "pkt_processor.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 见头文件定义
 * 修  改:
 *   时间 2014年01月11日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
ProcInfo PktProcessor::Process(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_data);

	ProcInfo ret = DoProcess(msg_type, msg_data);
	if (!(ret & PI_CHAIN_CONTINUE))
	{
		return ret; // 出错或者明确不再继续传递报文则返回
	}

	// 走到这里，说明需要往下继续处理
	if (!successor_)
	{
		DLOG(WARNING) << "Fail to find processor to process packet";
		return PI_RET_STOP;
	}
	else
	{
		return successor_->Process(msg_type, msg_data);
	}
}

}