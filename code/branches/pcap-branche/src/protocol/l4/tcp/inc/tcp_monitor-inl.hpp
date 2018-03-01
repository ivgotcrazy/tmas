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
#include "tmas_cfg.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年01月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
TcpMonitor<Next, Succ>::TcpMonitor() : tcp_switch_(false)
	, remove_inactive_conn_timeout_(30)
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
template<class Next, class Succ>
bool TcpMonitor<Next, Succ>::Init()
{
	// 由于在初始化TcpMonitor的时候，还未设置其Next/Succ，因此，必须将
	// 对象池的初始大小设为0，强制其在运行时生成对象，否则，Analyzer的
	// 继任处理器会设置失败。
	conn_info_pool_.reset(new ObjectPool<TcpConnInfo>(
		boost::bind(&TcpMonitor::ConstructTcpConnInfo, this),
		boost::bind(&TcpMonitor::DestructTcpConnInfo, this, _1),
		boost::bind(&TcpMonitor::ReinitTcpConnInfo, this, _1),
		0));

	tcp_timer_.reset(new FreeTimer(boost::bind(&TcpMonitor::OnTick, this), 5));
	TMAS_ASSERT(tcp_timer_);

	tcp_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 构造TcpConnInfo
 * 参  数: 
 * 返回值: TcpConnInfo*
 * 修  改:
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline TcpConnInfo* TcpMonitor<Next, Succ>::ConstructTcpConnInfo()
{
	TcpConnInfo* conn_info = new TcpConnInfo;

	TcpConnReorderTypeSP reorder(new TcpConnReorderType(this));
	TcpConnAnalyzerTypeSP analyzer(new TcpConnAnalyzerType(this));

	reorder->SetSuccProcessor(analyzer);
	analyzer->SetSuccProcessor(this->GetSuccProcessor());

	conn_info->processor = reorder;
	
	return conn_info;
}

/*-----------------------------------------------------------------------------
 * 描  述: 析构TcpConnInfo
 * 参  数: [in] conn_info TcpConnInfo*
 * 返回值: 
 * 修  改:
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::DestructTcpConnInfo(TcpConnInfo* conn_info)
{
	delete conn_info;
}

/*-----------------------------------------------------------------------------
 * 描  述: 重新初始化TcpConnInfo
 * 参  数: [in] conn_info TcpConnInfo*
 * 返回值: 
 * 修  改:
 *   时间 2014年03月23日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::ReinitTcpConnInfo(TcpConnInfo* conn_info)
{
	conn_info->abandoned = false;

	TMAS_ASSERT(conn_info->processor);

	conn_info->processor->ProcessMsg(MSG_REINITIALIZE, 0);
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
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::AbandonTcpConnection(const ConnId& conn_id)
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
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::RemoveTcpConnection(const ConnId& conn_id)
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	HashView& hash_view = tcp_conns_.get<0>();
	auto iter = hash_view.find(conn_id);
	if (iter == tcp_conns_.end())
	{
		DLOG(WARNING) << "Fail to remove connection | " << conn_id;
		return;
	}

	// 先通知上层
	this->PassMsgToSuccProcessor(MSG_TCP_CONN_CLOSED, &((*iter)->conn_id));

	hash_view.erase(iter); // 从容器中删除处理链

	DLOG(WARNING) << "Closed one tcp connection | " << conn_id;
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
template<class Next, class Succ>
inline ProcInfo TcpMonitor<Next, Succ>::DoProcessPkt(PktMsgSP& pkt_msg)
{
	// 非TCP报文由本处理链其他处理器继续处理
	if (pkt_msg->l3_pkt_info.l4_prot != IPPROTO_TCP)
	{
		return PI_NOT_PROCESSED;
	}

	if (!tcp_switch_) return PI_HAS_PROCESSED;

	if (!ParsePktTcpInfo(pkt_msg))
	{
		DLOG(WARNING) << "Fail to parse tcp info";
		return PI_HAS_PROCESSED;
	}

	PrintTcpInfo(pkt_msg);

	ProcessTcpPkt(pkt_msg);

	return PI_HAS_PROCESSED;
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
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::ProcessTcpPkt(PktMsgSP& pkt_msg)
{
	TcpConnInfoSP conn_info;

	// 获取或创建连接信息
	if (!GetTcpConnInfo(pkt_msg, conn_info))
	{
		return; // 内部处理已经有打印
	}

	TMAS_ASSERT(conn_info); 

	// 如果连接已经废弃，则报文无需再做处理
	if (conn_info->abandoned)
	{
		DLOG(WARNING) << "Received packet on abandoned connection";
		return;
	}

	TMAS_ASSERT(conn_info->processor);
	
	// 在连接中处理报文
	conn_info->processor->ProcessPkt(pkt_msg);
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
template<class Next, class Succ>
inline bool TcpMonitor<Next, Succ>::GetTcpConnInfo(const PktMsgSP& pkt_msg, 
	                                               TcpConnInfoSP& conn_info)
{
	const ConnId& conn_id = pkt_msg->l4_pkt_info.conn_id;

	boost::mutex::scoped_lock lock(conn_mutex_);

	// 先查找连接是否已经创建
	HashView& hash_view = tcp_conns_.get<0>();
	auto hash_iter = hash_view.find(conn_id);
	if (hash_iter != tcp_conns_.end())
	{
		hash_view.modify(hash_iter, LastPktInModifier(time(0)));
		conn_info = *hash_iter;

		DLOG(INFO) << "Tcp connection exists | " << conn_id;
		return true;
	}

	// FIN和RST报文不创建连接
	if (FIN_FLAG(pkt_msg) || RST_FLAG(pkt_msg)) return false;

	DLOG(INFO) << "Incoming a new tcp connection " << conn_id;
	
	// 从对象池中获取新的ConnInfo
	conn_info = conn_info_pool_->AllocObject();
	conn_info->conn_id = conn_id;
	conn_info->last_pkt_in = time(0);
	
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
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::OnTick()
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
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::ClearTimeoutConns()
{
	boost::mutex::scoped_lock lock(conn_mutex_);

	// 时间的获取必须在锁之后，否则可能出现时间逆转情况
	uint32 time_now = time(0);

	TimeView& time_view = tcp_conns_.get<1>();
	for (auto iter = time_view.begin(); iter != time_view.end(); )
	{
		TMAS_ASSERT(time_now >= (*iter)->last_pkt_in);

		if (time_now - (*iter)->last_pkt_in < remove_inactive_conn_timeout_)
		{
			return;
		}

		DLOG(WARNING) << "Removed one timeout connection | " << (*iter)->conn_id;

		// 先通知上层
		this->PassMsgToSuccProcessor(MSG_TCP_CONN_CLOSED, &((*iter)->conn_id));

		// 再从容器中删除
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
template<class Next, class Succ>
inline void TcpMonitor<Next, Succ>::PrintTcpInfo(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "### TCP || seq: " << SEQ_NUM(pkt_msg) 
		       << " ack: " << ACK_NUM(pkt_msg) 
			   << ", A:"   << ACK_FLAG(pkt_msg) 
			   << "|S:"    << SYN_FLAG(pkt_msg) 
		       << "|R:"    << RST_FLAG(pkt_msg) 
			   << "|F:"    << FIN_FLAG(pkt_msg)
			   << ", data: " << L4_DATA_LEN(pkt_msg);
}

}
