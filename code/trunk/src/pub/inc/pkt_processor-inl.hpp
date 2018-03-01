/*#############################################################################
 * 文件名   : pkt_processor-inl.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月23日
 * 文件描述 : PktProcessor类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_PROCESSOR_INL
#define BROADINTER_PKT_PROCESSOR_INL

#include "pkt_processor.hpp"

namespace BroadInter
{
/*-----------------------------------------------------------------------------
 * 描  述: 获取派生类指针
 * 参  数: 
 * 返回值: 派生类指针
 * 修  改: 
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline Drived* PktProcessor<Drived, Next, Succ>::GetDrived()
{
	return static_cast<Drived*>(this);
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理非报文消息
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 处理结果
 * 修  改: 
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline ProcInfo PktProcessor<Drived, Next, Succ>::ProcessMsg(MsgType msg_type, void* msg_data)
{
	// 先调用本处理器处理，不关心是否处理了
	GetDrived()->DoProcessMsg(msg_type, msg_data);

	if (next_processor_) // 无条件往下传递
	{
		next_processor_->DoProcessMsg(msg_type, msg_data);
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * 描  述: 调用继任处理器的非报文处理
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline void PktProcessor<Drived, Next, Succ>::PassMsgToSuccProcessor(MsgType msg_type, void* msg_data)
{
	if (succ_processor_)
	{
		succ_processor_->ProcessMsg(msg_type, msg_data);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置下一处理器
 * 参  数: [in] processor 处理器
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline void PktProcessor<Drived, Next, Succ>::SetNextProcessor(boost::shared_ptr<Next> processor)
{
	next_processor_ = processor;
}

/*-----------------------------------------------------------------------------
 * 描  述: 设置继任处理器
 * 参  数: [in] processor 处理器
 * 返回值: 
 * 修  改: 
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline void PktProcessor<Drived, Next, Succ>::SetSuccProcessor(boost::shared_ptr<Succ> processor)
{
	succ_processor_ = processor;
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取下一处理器
 * 参  数: 
 * 返回值: 处理器
 * 修  改: 
 *   时间 2014年03月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline boost::shared_ptr<Next> PktProcessor<Drived, Next, Succ>::GetNextProcessor() const
{
	return next_processor_;
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取继任处理器
 * 参  数: 
 * 返回值: 处理器
 * 修  改: 
 *   时间 2014年03月24日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline boost::shared_ptr<Succ> PktProcessor<Drived, Next, Succ>::GetSuccProcessor() const
{
	return succ_processor_;
}

/*-----------------------------------------------------------------------------
 * 描  述: 非报文消息的派生类默认实现，派生类如果无需处理消息，则不用实现此接口
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 处理结果
 * 修  改: 
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline ProcInfo PktProcessor<Drived, Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	return PI_HAS_PROCESSED;
}

}

#endif