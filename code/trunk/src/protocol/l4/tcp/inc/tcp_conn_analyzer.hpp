/*#############################################################################
 * 文件名   : tcp_conn_analyzer.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月09日
 * 文件描述 : TcpConnAnalyzer类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONN_ANALYZER
#define BROADINTER_TCP_CONN_ANALYZER

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "pkt_processor.hpp"
#include "connection.hpp"
#include "message.hpp"
#include "tcp_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: TCP连接分析
 * 作  者: teck_zhou
 * 时  间: 2014年01月10日
 ******************************************************************************/
template<class Next, class Succ>
class TcpConnAnalyzer : public PktProcessor<TcpConnAnalyzer<Next, Succ>, Next, Succ>
{
public:
	TcpConnAnalyzer(TcpMonitorType* tcp_monitor);

	//--- TODO: 待优化访问限制

	// 消息处理
	ProcInfo DoProcessMsg(MsgType msg_type, void* data);

private:
	//--------------------------------------------------------------------------

	struct HalfConnStat
	{
		HalfConnStat() : send_pkt_num(0), send_pkt_size(0) {}

		uint64 send_pkt_num;
		uint64 send_pkt_size;
	};

	//--------------------------------------------------------------------------

	struct TcpConnStat
	{
		TcpConnStat() : total_pkt_num(0), total_pkt_size(0), handshake_delay(0) {}

		uint64 total_pkt_num;
		uint64 total_pkt_size;
		uint32 handshake_delay;
		
		HalfConnStat half_conn_stat[PKT_DIR_BUTT];
	};

	//--------------------------------------------------------------------------

	struct HalfConnFsm
	{
		HalfConnFsm() : conn_role(TCR_UNKONWN), fsm_state(TFS_INIT) {}

		TcpConnRole conn_role;	// server or client
		TcpFsmState fsm_state;  // TCP状态机状态
	};

	//--------------------------------------------------------------------------

	struct TcpFsmInfo
	{
		TcpFsmInfo() : conn_state(0) {}

		uint8 conn_state;	// 0bit: 握手成功 1bit: 连接建立  2bit: 连接关闭
		
		HalfConnFsm half_conn_fsm[PKT_DIR_BUTT];
	};

	//--------------------------------------------------------------------------

private:
	void PktMsgProc(PktMsg* pkt_msg);

	// 重新初始化对象，以支持对象池化
	void ReInitialize();

	// 处理TCP状态机
	void ProcessTcpFsm(const PktMsg* pkt_msg);

	// 连接异常终止
	void ConnectionFailed(const ConnId& conn_id, ConnCloseReason reason);

	// 连接正常关闭
	void ConnectionClosed(const ConnId& conn_id);

	// 处理半连接的TCP状态机
	void ProcHalfConnFsm(const PktMsg* pkt_msg, 
		TcpFsmEvent fsm_event, HalfConnFsm& half_conn_fsm);

	// 处理连接相关统计
	void ProcessConnStat(const PktMsg* pkt_msg);

	// 查看连接的三次握手是否超时
	void CheckHandshakeTimeOut(const PktMsg* pkt_msg);

	// 记录三次握手超时的连接
	void ProcTcpHsTimeOut(const ConnId& conn_id);

	void RecordConnClosed(const ConnId& conn_id);

	void RecordHsTimeout(const ConnId& conn_id);

	void RecordConnFailed(const ConnId& conn_id);

private:
	TcpMonitorType* tcp_monitor_;

	// 第一个报文处理时间
	uint64 first_pkt_time_;			

	// 连接统计信息
	TcpConnStat conn_stat_;			

	// 连接的状态机信息
	TcpFsmInfo fsm_info_;			

	//是否记录了握手超时时间
	bool record_handshake_timeout_; 

	// 握手超时阈值
	uint32 handshake_timeout_;

	// 连接握手时延
	uint32 handshake_delay_;
};

}

#include "tcp_conn_analyzer-inl.hpp"

#endif
