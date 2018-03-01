/*#############################################################################
 * 文件名   : pkt_processor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : PktProcessor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_PROCESSOR
#define BROADINTER_PKT_PROCESSOR

#include <boost/noncopyable.hpp>

#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 处理器处理结果
 * 作  者: teck_zhou
 * 时  间: 2014年03月26日
 ******************************************************************************/
enum ProcInfo
{
	PI_NOT_PROCESSED,	// 未处理，指示上层处理器将消息传递给Next处理器处理
	PI_HAS_PROCESSED	// 已处理，指示上层处理器处理完成，直接返回即可
};

/*******************************************************************************
 * 描  述: 哑消息处理器，表示处理的终结，用于协助构建处理链。
 * 作  者: teck_zhou
 * 时  间: 2014年03月26日
 ******************************************************************************/
class None : public boost::noncopyable
{
public:
	ProcInfo ProcessPkt(PktMsgSP& pkt_msg) { return PI_HAS_PROCESSED; }
	ProcInfo ProcessMsg(MsgType msg_type, void* msg_data) { return PI_HAS_PROCESSED; }

	ProcInfo DoProcessPkt(PktMsgSP& pkt_msg) { return PI_HAS_PROCESSED; }
	ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data) { return PI_HAS_PROCESSED; }
};

/*******************************************************************************
 * 描  述: 消息处理器基类
 * 作  者: teck_zhou
 * 时  间: 2014年01月11日
 ******************************************************************************/
template<class Drived, class Next, class Succ>
class PktProcessor : public boost::noncopyable
{
public:
	// 析构函数
	virtual ~PktProcessor() {}

	// 处理报文消息
	inline ProcInfo ProcessPkt(PktMsgSP& pkt_msg);

	// 处理非报文消息
	inline ProcInfo ProcessMsg(MsgType msg_type, void* msg_data);

	// 支持手动控制报文传递到继任处理器处理
	inline void PassPktToSuccProcessor(PktMsgSP& pkt_msg);

	// 支持手动控制消息传递到继任处理器处理
	inline void PassMsgToSuccProcessor(MsgType msg_type, void* msg_data);

	// 设置下一处理器
	inline void SetNextProcessor(boost::shared_ptr<Next> processor);

	// 设置继任处理器
	inline void SetSuccProcessor(boost::shared_ptr<Succ> processor);

	// 获取下一处理器处理接口
	inline boost::shared_ptr<Next> GetNextProcessor() const;

	// 获取继任处理器处理接口
	inline boost::shared_ptr<Succ> GetSuccProcessor() const;

private:
	// 获取派生类对象指针
	inline Drived* GetDrived();

	// 提供默认实现，派生类可以不实现此接口
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// 提供默认实现，派生类可以不实现此接口
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* msg_data);

private:

	// 当前处理器未处理消息，需要将消息传递下去的下一个处理器
	boost::shared_ptr<Next> next_processor_;

	// 当前处理器处理完消息后，需要将消息传递下去的继任处理器
	boost::shared_ptr<Succ> succ_processor_;
};

}

#include "pkt_processor-inl.hpp"

#endif