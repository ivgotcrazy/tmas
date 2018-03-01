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

namespace BroadInter
{

using namespace boost::multi_index;

//--------------------------------------------------------------------------

struct TcpConnInfo
{
	TcpConnInfo() : abandoned(false) {}

	ConnId conn_id;					// 连接标识
	bool abandoned;					// 连接是否已经废弃，不再处理报文
	uint32 first_pkt_in;			// 连接第一个报文到达时间
	uint32 last_pkt_in;				// 连接最后一个报文达到时间
	PktProcessorSP pkt_processor;	// 连接报文处理器
};

typedef boost::shared_ptr<TcpConnInfo> TcpConnInfoSP;

/*******************************************************************************
 * 描  述: 连接信息对象池
 * 作  者: teck_zhou
 * 时  间: 2014年03月21日
 ******************************************************************************/
class TcpConnInfoPool
{
public:
	TcpConnInfoPool(TcpMonitor* monitor, size_t pool_size = EXPAND_SIZE);

	TcpConnInfoSP Alloc();
	void Free(const TcpConnInfoSP& conn_info);

private:
	enum { EXPAND_SIZE = 128 };

	void Expand(size_t expand_size = EXPAND_SIZE);

private:
	boost::mutex pool_mutex_;

	std::list<TcpConnInfoSP> conn_info_list_;

	TcpMonitor* tcp_monitor_;
};

/*******************************************************************************
 * 描  述: TCP报文处理类
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
class TcpMonitor : public PktProcessor
{
public:
	TcpMonitor(const PktProcessorSP& processor);

	bool Init();

	void AbandonTcpConnection(const ConnId& conn_id);

	void RemoveTcpConnection(const ConnId& conn_id);

	void TransferPktToL7(const PktMsgSP& pkt_msg);

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

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
	void OnTick();
	void ResolveTcpInfo(PktMsgSP& pkt_msg);

	PktProcessorSP GetTcpConnInfo(const ConnId& conn_id);
	PktProcessorSP ConstructPktProcessor(const ConnId& conn_id);

	bool GetTcpConnInfo(const PktMsgSP& pkt_msg, TcpConnInfoSP& conn_info);
	
	void ProcessTcpPkt(const PktMsgSP& pkt_msg);
	void PrintTcpInfo(const PktMsgSP& pkt_msg);

	void ClearTimeoutConns();

private:
	bool tcp_switch_;

	TcpConnContainer tcp_conns_;
	boost::mutex conn_mutex_;

	// 清理超时连接定时器
	boost::scoped_ptr<FreeTimer> tcp_timer_;

	TcpMonitorStat tcp_monitor_stat_;

	uint32 remove_inactive_conn_timeout_; 

	PktProcessorSP l4_monitor_;

	TcpConnInfoPool conn_info_pool_;
};

}


#endif