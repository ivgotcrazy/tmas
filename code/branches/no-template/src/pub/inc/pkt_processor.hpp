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
#include <boost/enable_shared_from_this.hpp>

#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "message.hpp"

namespace BroadInter
{

// 报文处理模型：
//=============================================================================
// --->A-------->B ------------>C-------->I
//               |              |
//              \|/            \|/
//			     D--->E--->F    G
//               |
//              \|/
//               H
//=============================================================================
// 报文处理规则：报文从左到右，从上到下单向流动。
// 
// 组成元素：
// 处理节点：负责报文处理，并需要根据处理结果进行报文传递控制，比如A、B、C等。
// 处 理 链：将处理节点的一种线性结构，比如A-B-C-I、D-H等。
// 
// 处理规则:
// 1) 每个处理节点必属于某一处理链，比如D节点属于D-H处理链。
// 2) 处理节点可以包含子处理链，比如B节点包含D-H处理链。
// 3) 一个处理节点只能属于一条处理链，比如D节点属于B-D-H处理链，不属于E-F处理链。

// 接口ProcessPkt的返回值类型，包含调用者进行报文传递控制的所需信息，支持扩展。
typedef uint8 ProcInfo;		

//---------------------------- 返回值包含信息定制 -----------------------------

// PI: Process Information

#define PI_CHAIN_CONTINUE	0x01	// 第0个比特表示处理链是否需要将报文往下传递
#define PI_L4_CONTINUE		0x02	// 第1个比特表示L4是否需要将报文继续往下传递
#define PI_L7_CONTINUE		0x04	// 第2个比特表示L7是否需要将报文继续往下传递

//------------------------------- 返回值预定义 --------------------------------

#define PI_RET_STOP		0x00	// 所有处理停止，立即返回

/*******************************************************************************
 * 描  述: 消息处理器基类
 * 作  者: teck_zhou
 * 时  间: 2014年01月11日
 ******************************************************************************/
class PktProcessor : public boost::enable_shared_from_this<PktProcessor>
{
public:
	virtual ~PktProcessor() {}

	virtual void ReInit() {} // TODO: 此处是为了兼容处理器池化，ugly

	ProcInfo Process(MsgType msg_type, VoidSP msg_data);

	void set_successor(const PktProcessorSP& processor) { successor_ = processor; }
	PktProcessorSP get_successor() const { return successor_; }

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) = 0;

protected:
	PktProcessorSP successor_; // 继任处理器
};


}

#endif