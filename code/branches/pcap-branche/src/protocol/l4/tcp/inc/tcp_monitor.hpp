/*#############################################################################
 * 文件名   : tcp_monitor.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月02日
 * 文件描述 : TcpMonitor类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_MONITOR
#define BROADINTER_TCP_MONITOR

#include <list>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "timer.hpp"
#include "tcp_typedef.hpp"
#include "object_pool.hpp"

namespace BroadInter
{

using namespace boost::multi_index;

/*******************************************************************************
 * 描  述: TCP连接信息，此对象代表一个TCP连接
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
struct TcpConnInfo
{
	TcpConnInfo() : abandoned(false) {}

	ConnId conn_id;					// 连接标识
	bool   abandoned;				// 连接是否已经废弃，如果废弃则不再处理报文
	uint32 first_pkt_in;			// 连接第一个报文到达时间
	uint32 last_pkt_in;				// 连接最后一个报文达到时间
	TcpConnReorderTypeSP processor;	// 报文处理器
};

typedef boost::shared_ptr<TcpConnInfo> TcpConnInfoSP;

/*******************************************************************************
 * 描  述: TCP报文处理类
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
template<class Next, class Succ>
class TcpMonitor : public PktProcessor<TcpMonitor<Next, Succ>, Next, Succ>
{
public:
	TcpMonitor();

	bool Init();

	//--- TODO: 待优化访问限制

	// 报文消息处理
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// 提供给连接处理器回调，连接异常终止
	inline void AbandonTcpConnection(const ConnId& conn_id);

	// 提供给连接处理器回调，连接正常关闭
	inline void RemoveTcpConnection(const ConnId& conn_id);

	// 创建/销毁/重新初始化连接处理对象，用来支持池化
	inline TcpConnInfo* ConstructTcpConnInfo();
	inline void DestructTcpConnInfo(TcpConnInfo* conn_info);
	inline void ReinitTcpConnInfo(TcpConnInfo* conn_info);

private:
	
	//--------------------------------------------------------------------------

	typedef multi_index_container<
		TcpConnInfoSP,
		indexed_by<
			hashed_unique<
				member<TcpConnInfo, ConnId, &TcpConnInfo::conn_id>
			>,
			ordered_non_unique<
				member<TcpConnInfo, uint32, &TcpConnInfo::last_pkt_in>
			>
		> 
	>TcpConnContainer;
	typedef TcpConnContainer::nth_index<0>::type HashView;
	typedef TcpConnContainer::nth_index<1>::type TimeView;

	//--------------------------------------------------------------------------

	struct LastPktInModifier
	{
		LastPktInModifier(uint32 time_new) : new_time(time_new) {}

		void operator()(TcpConnInfoSP& conn_info)
		{
			conn_info->last_pkt_in = new_time;
		}

		uint32 new_time;
	};

	//--------------------------------------------------------------------------

	struct TcpMonitorStat
	{
		TcpMonitorStat() : closed_conn_for_handshake_fail(0)
			, closed_conn_for_cached_too_many_pkt(0) {}

		uint64 closed_conn_for_handshake_fail;
		uint64 closed_conn_for_cached_too_many_pkt;
	};
	
private:
	// 清除超时连接定时器处理
	inline void OnTick();

	// 处理TCP报文
	inline void ProcessTcpPkt(PktMsgSP& pkt_msg);

	// 获取TCP连接处理信息结构
	inline bool GetTcpConnInfo(const PktMsgSP& pkt_msg, TcpConnInfoSP& conn_info);

	// 打印报文的TCP层信息
	inline void PrintTcpInfo(const PktMsgSP& pkt_msg);

	// 清除超时连接
	inline void ClearTimeoutConns();

private:
	// tcp协议开关
	bool tcp_switch_;

	// 保存所有TCP连接
	TcpConnContainer tcp_conns_;
	boost::mutex conn_mutex_;

	// 清理超时连接定时器
	boost::scoped_ptr<FreeTimer> tcp_timer_;

	// 统计信息
	TcpMonitorStat tcp_monitor_stat_;

	// 超过设定时间未收到任何报文，则认为连接超时
	uint32 remove_inactive_conn_timeout_;

	// 连接处理对象池
	boost::shared_ptr<ObjectPool<TcpConnInfo> > conn_info_pool_;
};

}

#include "tcp_monitor-inl.hpp"

#endif