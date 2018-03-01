/*#############################################################################
 * 文件名   : l7_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : L7Monitor类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "l7_monitor.hpp"
#include "tmas_assert.hpp"
#include "http_monitor.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool L7Monitor::Init()
{
    HttpMonitor* http_monitor = new HttpMonitor;
    pkt_processor_.reset(http_monitor);

    if (!http_monitor->Init())
    {
        DLOG(INFO) << "Fail to init HTTP protocol monitor.";
        return false;
    }

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值:
 * 修  改: 
 *   时间 2014年01月08日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
ProcInfo L7Monitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_data);
	TMAS_ASSERT(pkt_processor_);

	// 调用子处理链
	pkt_processor_->Process(msg_type, msg_data);

	// 由于L7Monitor已经是主处理链的末端，不需要再将报文向下传递
	return PI_RET_STOP;
}

}
