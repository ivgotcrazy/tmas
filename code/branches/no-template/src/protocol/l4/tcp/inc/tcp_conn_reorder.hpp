/*#############################################################################
 * 文件名   : tcp_conn_reorder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月09日
 * 文件描述 : TcpConnReorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONN_REORDER
#define BROADINTER_TCP_CONN_REORDER

#include <list>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "tcp_fsm.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: TCP报文重排序器
 * 作  者: teck_zhou
 * 时  间: 2014年01月10日
 ******************************************************************************/
class TcpConnReorder : public PktProcessor
{
public:
	TcpConnReorder(TcpMonitor* tcp_monitor);
	~TcpConnReorder();

	virtual void ReInit() override;

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	//--------------------------------------------------------------------------
	enum PktOrderType
	{
		POT_IN_ORDER,
		POT_OUT_OF_ORDER,
		POT_IGNORE,
		POT_RETRANSMIT
	};
	//--------------------------------------------------------------------------
	typedef std::list<PktMsgSP> PktMsgList;
	typedef PktMsgList::iterator PktMsgIter;
	//--------------------------------------------------------------------------
	struct HalfConnInfo
	{
		HalfConnInfo() : sent_seq_num(0), sent_ack_num(0)
			, next_seq_num(0), next_ack_num(0), send_pkt_num(0)
		{
		}

		uint32 sent_seq_num; // 已发送的发送号
		uint32 sent_ack_num; // 已发送的确认号

		uint32 next_seq_num; // 下一个报文的发送号
		uint32 next_ack_num; // 下一个报文的确认号

		uint64 send_pkt_num;
		PktMsgList cached_pkt; // 无序报文缓存
	};
	//--------------------------------------------------------------------------
	struct TcpConnInfo
	{
		TcpConnInfo() : started(false) {}

		bool started; // 三次握手的第一个报文是否已经到达

		HalfConnInfo half_conn[2]; // 两个方向的连接信息
	};
	//--------------------------------------------------------------------------

private:
	PktOrderType GetPktOrderType(const PktMsgSP& pkt_msg);
	bool IsFirstPktOf3WHS(const PktMsgSP& pkt_msg);
	bool IsSecondPktOf3WHS(const PktMsgSP& pkt_msg);
	void TryProcOrderedPkt(const PktMsgSP& pkt_msg);
	void ProcessOrderedPkt(const PktMsgSP& pkt_msg);
	void ProcessUnorderedPkt(const PktMsgSP& pkt_msg);
	void TryProcCachedPkt();
	void AddUnorderedPkt(const PktMsgSP& pkt_msg);
	bool ProcOneCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter);
	void RefreshTcpConn(const PktMsgSP& pkt_msg);
	void ProcHalfConnCachedPkt(PktMsgList& pkt_list, PktMsgIter& pkt_iter,
		                       bool& local_half_flag, bool& other_half_flag);
	void InsertUnorderedPktToList(const PktMsgSP& pkt_msg, PktMsgList& pkt_list);
	void ReInitHalfConn(HalfConnInfo& half_conn);

private:
	TcpMonitor* tcp_monitor_;

	TcpConnInfo conn_info_;

	// 此锁用来保证同一时刻只有一个线程
	// 在处理此TCP连接的报文。
	boost::mutex reorder_mutex_;

	uint32 max_cached_unordered_pkt_;
	uint32 max_cached_pkt_before_handshake_;
};

}

#endif

