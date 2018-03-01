/*#############################################################################
 * 文件名   : tcp_monitor.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : TcpMonitor类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include "tcp_monitor.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "tmas_config_parser.hpp"
#include "tcp_conn_reorder.hpp"
#include "tcp_conn_analyzer.hpp"
#include "pkt_parser.hpp"
#include "tmas_config_parser.hpp"

namespace BroadInter
{

#define MAX_ALIVE_TIME_WITHOUT_PKT	30 //秒

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] tcp_monitor 所属TcpMonitor
 *         [in] pool_size 对象池大小
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
TcpConnInfoPool::TcpConnInfoPool(TcpMonitor* monitor, size_t pool_size /* = EXPAND_SIZE */)
	: tcp_monitor_(monitor)
{
	Expand(pool_size);
}

/*-----------------------------------------------------------------------------
 * 描  述: 申请对象
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
TcpConnInfoSP TcpConnInfoPool::Alloc()
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	if (conn_info_list_.empty())
	{
		Expand();
		DLOG(WARNING) << "Expand tcp connection processors";
	}

	TcpConnInfoSP conn_info = conn_info_list_.front();
	conn_info_list_.pop_front();
	
	// 重新初始化
	conn_info->pkt_processor->ReInit();
	conn_info->pkt_processor->get_successor()->ReInit();

	return conn_info;
}

/*-----------------------------------------------------------------------------
 * 描  述: 释放对象
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpConnInfoPool::Free(const TcpConnInfoSP& conn_info)
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	conn_info_list_.push_back(conn_info);
}

/*-----------------------------------------------------------------------------
 * 描  述: 扩充对象池大小
 * 参  数: [in] expand_size 扩充大小
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpConnInfoPool::Expand(size_t expand_size /* = EXPAND_SIZE */)
{
	for (size_t i = 0; i < expand_size; i++)
	{
		TcpConnInfoSP conn_info(new TcpConnInfo);

		PktProcessorSP processor(new TcpConnReorder(tcp_monitor_));
		processor->set_successor(PktProcessorSP(new TcpConnAnalyzer(tcp_monitor_)));

		conn_info->pkt_processor = processor;

		conn_info_list_.push_back(conn_info);
	}
}

///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
TcpMonitor::TcpMonitor(const PktProcessorSP& processor) 
	: tcp_switch_(false)
	, remove_inactive_conn_timeout_(30)
	, l4_monitor_(processor)
	, conn_info_pool_(this)
{
	GET_TMAS_CONFIG_BOOL("global.protocol.tcp", tcp_switch_);
	if (!tcp_switch_)
	{
		DLOG(WARNING) << "Tcp monitor is closed";
	}

	GET_TMAS_CONFIG_INT("global.tcp.remove-inactive-connection-timeout", 
		                remove_inactive_conn_timeout_);
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年02月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpMonitor::Init()
{
	tcp_timer_.reset(new FreeTimer(boost::bind(&TcpMonitor::OnTick, this), 5));

	TMAS_ASSERT(tcp_timer_);

	tcp_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 将报文传递到L7
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::TransferPktToL7(const PktMsgSP& pkt_msg)
{
	l4_monitor_->get_successor()->Process(MSG_PKT, VOID_SHARED(pkt_msg));
}

/*-----------------------------------------------------------------------------
 * 描  述: 废弃连接
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改:
 *   时间 2014年03月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::AbandonTcpConnection(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	HashView& hash_view = tcp_conns_.get<0>();
	auto iter = hash_view.find(conn_id);
	if (iter == tcp_conns_.end())
	{
		DLOG(WARNING) << "Fail to abandon connection | " << conn_id;
		return;
	}

	(*iter)->abandoned = true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 关闭四次握手正常关闭报文
 * 参  数: [in] conn_id 连接标识
 * 返回值: 
 * 修  改:
 *   时间 2014年01月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::RemoveTcpConnection(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	HashView& hash_view = tcp_conns_.get<0>();
	auto iter = hash_view.find(conn_id);
	if (iter == tcp_conns_.end())
	{
		DLOG(WARNING) << "Fail to remove connection | " << conn_id;
		return;
	}

	conn_info_pool_.Free(*iter);

	hash_view.erase(conn_id);
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理报文
 * 参  数: [in] msg_type 消息类型
 *         [in] msg_data 消息数据
 * 返回值: 报文是否需要继续处理
 * 修  改:
 *   时间 2014年01月02日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
ProcInfo TcpMonitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	if (!tcp_switch_) // TCP处理器开关关闭，由本处理链其他处理器处理
	{
		return PI_CHAIN_CONTINUE;
	}

	TMAS_ASSERT(msg_type == MSG_PKT);
	PktMsgSP pkt_msg = boost::static_pointer_cast<PktMsg>(msg_data);

	// 非TCP报文由本处理链其他处理器继续处理
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_TCP)
	{
		return PI_CHAIN_CONTINUE;
	}

	if (!ParsePktTcpInfo(pkt_msg))
	{
		DLOG(WARNING) << "Fail to parse tcp info";
		return PI_RET_STOP;
	}

	PrintTcpInfo(pkt_msg);

	ProcessTcpPkt(pkt_msg); // 处理报文
	
	return PI_RET_STOP; // 报文全部通过手动往上层传递
}

/*-----------------------------------------------------------------------------
 * 描  述: 处理TCP报文
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年01月27日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::ProcessTcpPkt(const PktMsgSP& pkt_msg)
{
	TcpConnInfoSP conn_info;

	// 获取或创建连接信息
	if (!GetTcpConnInfo(pkt_msg, conn_info))
	{
		return; // 内部处理已经有打印
	}

	TMAS_ASSERT(conn_info);

	// 无条件刷新连接最后一个报文到达时间
	conn_info->last_pkt_in = time(0); 

	// 如果连接已经废弃，则报文无需再做处理
	if (conn_info->abandoned)
	{
		DLOG(WARNING) << "Received packet on abandoned connection";
		return;
	}

	TMAS_ASSERT(conn_info->pkt_processor);

	// 在连接中处理报文
	conn_info->pkt_processor->Process(MSG_PKT, VOID_SHARED(pkt_msg));
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取TCP连接
 * 参  数: [in] pkt_msg 报文消息
 *         [out] conn_info 连接信息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年01月11日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool TcpMonitor::GetTcpConnInfo(const PktMsgSP& pkt_msg, TcpConnInfoSP& conn_info)
{
	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	boost::mutex::scoped_lock lock(conn_mutex_);

	// 先查找连接是否已经创建
	HashView& hash_view = tcp_conns_.get<0>();
	auto hash_iter = hash_view.find(conn_id);
	if (hash_iter != tcp_conns_.end())
	{
		DLOG(INFO) << "Tcp connection exists | " << conn_id;
		conn_info = *hash_iter;
		return true;
	}

	// FIN和RST报文不创建连接
	if (FIN_FLAG(pkt_msg) || RST_FLAG(pkt_msg)) return false;

	DLOG(INFO) << "Incoming a new tcp connection " << conn_id;
	
	// 从对象池中获取新的ConnInfo
	conn_info = conn_info_pool_.Alloc();
	conn_info->conn_id = conn_id;
	
	// 将新ConnInfo插入容器
	TcpConnContainer::value_type insert_value(conn_info);
	if (!hash_view.insert(insert_value).second)
	{
		LOG(ERROR) << "Fail to insert connection";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 定时器处理，当前处理没有考虑效率，后续需要优化
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年02月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::OnTick()
{
	ClearTimeoutConns();
}

/*-----------------------------------------------------------------------------
 * 描  述: 如果很久都没有收到报文，则删除连接
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::ClearTimeoutConns()
{
	uint32 time_now = time(0);

	boost::mutex::scoped_lock lock(conn_mutex_);

	TimeView& time_view = tcp_conns_.get<1>();
	for (auto iter = time_view.begin(); iter != time_view.end(); )
	{
		TMAS_ASSERT(time_now >= (*iter)->last_pkt_in);
		if (time_now - (*iter)->last_pkt_in < remove_inactive_conn_timeout_)
		{
			return;
		}

		DLOG(WARNING) << "Removed one timeout connection | " << (*iter)->conn_id;

		conn_info_pool_.Free(*iter);

		iter = time_view.erase(iter);
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 打印TCP信息
 * 参  数: [in] pkt_msg 报文消息
 * 返回值: 
 * 修  改:
 *   时间 2014年02月11日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void TcpMonitor::PrintTcpInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### TCP || seq: " << SEQ_NUM(pkt_msg) 
		      << " ack: " << ACK_NUM(pkt_msg) 
			  << ", A:" << ACK_FLAG(pkt_msg) 
			  << "|S:" << SYN_FLAG(pkt_msg) 
		      << "|R:" << RST_FLAG(pkt_msg) 
			  << "|F:" << FIN_FLAG(pkt_msg)
			  << ", data: " << L4_DATA_LEN(pkt_msg);
}

}
