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
template<class Next, class Succ>
class TcpConnReorder : public PktProcessor<TcpConnReorder<Next, Succ>, Next, Succ>
{
public:
	TcpConnReorder(TcpMonitorType* tcp_monitor);

	~TcpConnReorder();

	//--- TODO: 待优化访问限制

	// 报文消息处理
	inline ProcInfo DoProcessPkt(PktMsgSP& pkt_msg);

	// 非报文消息处理
	inline ProcInfo DoProcessMsg(MsgType msg_type, void* data);

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
	// 重新初始化，用以支持对象池化
	inline void Reinitialize();

	// 重新初始化半连接状态，用以支持对象池化
	inline void ReInitHalfConn(HalfConnInfo& half_conn);

	// 判断报文是否有序
	inline PktOrderType GetPktOrderType(const PktMsgSP& pkt_msg);

	// 是否是TCP三次握手的第一个报文
	inline bool IsFirstPktOf3WHS(const PktMsgSP& pkt_msg);

	// 是否是TCP三次握手的第二个报文
	inline bool IsSecondPktOf3WHS(const PktMsgSP& pkt_msg);

	// 仅处理有序报文
	inline void ProcessOrderedPkt(PktMsgSP& pkt_msg);

	// 处理缓存报文
	inline void TryProcCachedPkt();

	// 先处理有序报文，并继续处理缓存报文
	inline void TryProcOrderedPkt(PktMsgSP& pkt_msg);

	// 处理乱序报文
	inline void ProcessUnorderedPkt(PktMsgSP& pkt_msg);

	// 将乱序报文缓存起来
	inline void AddUnorderedPkt(const PktMsgSP& pkt_msg);

	// 处理完一个有序报文后，需要刷新连接状态
	inline void RefreshTcpConn(const PktMsgSP& pkt_msg);

	// 处理一个有序的缓存报文
	inline bool ProcOneCachedPkt(PktMsgList& pkt_list, 
		                         PktMsgIter& pkt_iter);

	// 将乱序报文插入缓存容器
	inline void InsertUnorderedPktToList(const PktMsgSP& pkt_msg, 
										 PktMsgList& pkt_list);

	// 处理某一个半连接的缓存报文
	inline void ProcHalfConnCachedPkt(PktMsgList& pkt_list, 
									  PktMsgIter& pkt_iter,
									  bool& local_half_flag, 
									  bool& other_half_flag);

	// 重构TCP连接，使得数据流能够继续流动
	inline void RefactorTcpConn();

	// 通过刷新连接状态参数，使得缓存报文变得有序
	inline void MakeChacedPktOrdered();

	// 获取双向连接缓存报文总数
	inline uint32 GetTotalCachedPktNum();

	// 握手失败处理
	inline void HandshakeFailed(const PktMsgSP& pkt_msg);

private:
	TcpMonitorType* tcp_monitor_;

	// 重复报文检测、报文重排序等所需状态信息
	TcpConnInfo conn_info_;

	// 保证同一时刻只有一个线程处理某一个TCP连接的报文。
	boost::mutex reorder_mutex_;

	// 一个连接不能缓存太多无序报文
	uint32 max_cached_unordered_pkt_;

	// 如果丢失了三次握手报文，则不再重构此连接
	uint32 max_cached_pkt_before_handshake_;

	// 可能出现连接关闭后，还有后续报文需要处理的情况
	// 可能多个线程都阻塞在连接处理入口，其中一个线程
	// 将连接关闭了，其他线程再进入，不应该再处理报文
	bool conn_closed_;
};

}

#include "tcp_conn_reorder-inl.hpp"

#endif

