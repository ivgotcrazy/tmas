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

class TcpMonitor;

/*******************************************************************************
 * 描  述: TCP连接分析
 * 作  者: teck_zhou
 * 时  间: 2014年01月10日
 ******************************************************************************/
class TcpConnAnalyzer : public PktProcessor
{
public:
	TcpConnAnalyzer(TcpMonitor* tcp_monitor);

	virtual void ReInit() override;

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

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
	void ProcessTcpFsm(const PktMsgSP& pkt_msg);

	void ConnectionFailed(const ConnId& conn_id, ConnCloseReason reason);
	void ConnectionClosed(const ConnId& conn_id);

	void ProcHalfConnFsm(const PktMsgSP& pkt_msg, 
		                 TcpFsmEvent fsm_event, 
						 HalfConnFsm& half_conn_fsm);
	void ProcessConnStat(const PktMsgSP& pkt_msg);

	void CheckHandshakeTimeOut(const PktMsgSP& pkt_msg);

	void ProcTcpHsTimeOut(uint32 times, const ConnId& conn_id);

private:
	TcpMonitor* tcp_monitor_;

	ptime first_pkt_time_;  // 第一个报文处理时间

	TcpConnStat conn_stat_;	// 连接统计

	TcpFsmInfo fsm_info_;	// 状态机信息

	bool record_handshake_timeout_; //是否记录了握手超时时间

	uint32 complete_handshake_timeout_; 
};

}

#endif