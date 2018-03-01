/*#############################################################################
 * 文件名   : tcp_conn_analyzer_test.cpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年03月5日
 * 文件描述 : TcpConnAnalyzer 实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/
#include <glog/logging.h>
#include <fstream>

#include "tcp_conn_analyzer_test.hpp"

#include "pkt_parser.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "tcp_monitor.hpp"


namespace BroadInter
{
#define CONN_CLOSED			0x04
	
#define DEFAULT_SRC_IP		0xc0a80001
#define DEFAULT_DST_IP		0xc0a80002
#define DEFAULT_SRC_PORT	3333
#define DEFAULT_DST_PORT	4444

#define SET_PKT_DEFAULT_PARA(constructor, src_ip, src_port, dst_ip, dst_port) \
	constructor.GetIpLayer()->SetSrcIpAddr(src_ip); \
	constructor.GetIpLayer()->SetDstIpAddr(dst_ip); \
	constructor.SetSrcPort(src_port); \
	constructor.SetDstPort(dst_port);


//==============================================================================
// Test class implementation
//==============================================================================

TcpConnAnalyzerTest::TcpConnAnalyzerTest() 
	: tcp_conn_analyzer_(nullptr)
	, tcp_monitor_successor_(new PktProcessorStub())
	, l4_monitor_(new PktProcessorStub())
	
{
	TmasConfigParser::GetInstance().Init();
}

TcpConnAnalyzerTest::~TcpConnAnalyzerTest()
{
}

void TcpConnAnalyzerTest::SetUp()
{
	ConnId id(6, 0xc0a80001, 3333, 0xc0a80002, 4444);

	tcp_conn_analyzer_ = new TcpConnAnalyzer(new TcpMonitor(tcp_monitor_successor_), 
							l4_monitor_, id);
}

void TcpConnAnalyzerTest::TearDown()
{
	delete tcp_conn_analyzer_;
}

/*------------------------------------------------------------------------------
 * 描述: 解析ETH/IP/TCP三层协议信息
 *----------------------------------------------------------------------------*/
bool TcpConnAnalyzerTest::ParsePktInfo(PktMsgSP& pkt_msg)
{
	if (!ParsePktEthInfo(pkt_msg)) return false;

	if (!ParsePktIpInfo(pkt_msg)) return false;

	if (!ParsePktTcpInfo(pkt_msg)) return false;

	LOG(INFO) << "Fin Flag : " << pkt_msg->l4_pkt_info.fin_flag;
	
	return true;
}

/*------------------------------------------------------------------------------
 * 描述: 构建三次握手的第一个报文
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct3WHSFirstPkt()
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP参数
	tcp_constructor.SetSeqNum(10001);
	tcp_constructor.SetAckNum(0);

	tcp_constructor.SetSynFlag(true);

	// 获取PktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// 解析报文
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * 描述: 构建三次握手的第二个报文
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct3WHSSecondPkt()
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// 设置TCP层参数
	tcp_constructor.SetSeqNum(20001);
	tcp_constructor.SetAckNum(10002);

	tcp_constructor.SetSynFlag(true);
	tcp_constructor.SetAckFlag(true);

	// 获取PktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// 解析报文
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * 描述: 构建三次握手的第三个报文
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct3WHSThirdPkt()
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_constructor.SetSeqNum(10002);
	tcp_constructor.SetAckNum(20002);

	tcp_constructor.SetAckFlag(true);

	// 获取PktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// 解析报文
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * 描述: 构建四次解手的第一个报文
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct4WHSFirstPkt(uint32 src_ip, uint16 src_port,
															uint32 dst_ip, uint16 dst_port)
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, src_ip, 
		src_port, dst_ip, dst_port);

	// 设置TCP层参数
	tcp_constructor.SetSeqNum(20001);
	tcp_constructor.SetAckNum(30001);

	tcp_constructor.SetFinFlag(true);
	tcp_constructor.SetAckFlag(true);

	// 获取PktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// 解析报文
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * 描述: 构建四次解手的第二个报文
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct4WHSSecondPkt(uint32 src_ip, uint16 src_port,
															uint32 dst_ip, uint16 dst_port)
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, src_ip, 
		src_port, dst_ip, dst_port);

	// 设置TCP层参数
	tcp_constructor.SetSeqNum(30002);
	tcp_constructor.SetAckNum(20002);

	tcp_constructor.SetFinFlag(true);
	tcp_constructor.SetAckFlag(true);

	// 获取PktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// 解析报文
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}



/*------------------------------------------------------------------------------
 * 描述: 完成TCP三次握手
 *----------------------------------------------------------------------------*/
void TcpConnAnalyzerTest::AccomplishTcp3WHS()
{
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_;

	//--------------------------
	//--- 三次握手第一个报文
	//--------------------------
	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_first_pkt));

	uint8 sender = first_pkt->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);

	ASSERT_EQ(TFS_SYN_SENT, fsm_info.half_conn_fsm[sender].fsm_state);
	ASSERT_EQ(TFS_SYN_RCV, fsm_info.half_conn_fsm[recver].fsm_state);

	//--------------------------
	//--- 三次握手第二个报文
	//--------------------------
	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);

	ASSERT_EQ(TFS_ESTABLISHED, fsm_info.half_conn_fsm[recver].fsm_state);
	

	//--------------------------
	//--- 三次握手第三个报文
	//--------------------------
	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_third_pkt));

	sender = third_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	
	ASSERT_EQ(TFS_ESTABLISHED, fsm_info.half_conn_fsm[recver].fsm_state);

	delete[] first_pkt->pkt.buf;
	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 三次握手 非超时
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_1)
{
	AccomplishTcp3WHS();
}

#if 1
/*------------------------------------------------------------------------------
 * 描述: 三次握手 超时
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_2)
{
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_;

	//--------------------------
	//--- 三次握手第一个报文
	//--------------------------
	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_first_pkt));

	uint8 sender = first_pkt->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);

	ASSERT_EQ(TFS_SYN_SENT, fsm_info.half_conn_fsm[sender].fsm_state);
	ASSERT_EQ(TFS_SYN_RCV, fsm_info.half_conn_fsm[recver].fsm_state);

	//--------------------------
	//--- 三次握手第二个报文
	//--------------------------
	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);

	ASSERT_EQ(TFS_ESTABLISHED, fsm_info.half_conn_fsm[recver].fsm_state);

	//超时时间是5秒
	boost::this_thread::sleep(boost::posix_time::seconds(5));

	//--------------------------
	//--- 三次握手第三个报文
	//--------------------------
	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_third_pkt));

	sender = third_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	
	ASSERT_EQ(TFS_ESTABLISHED, fsm_info.half_conn_fsm[recver].fsm_state);

	delete[] first_pkt->pkt.buf;
	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
}
#endif

/*------------------------------------------------------------------------------
 * 描述: 发送Fin报文， 客户端发送fin 进入 fin_wait_1 状态
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_3)
{
	AccomplishTcp3WHS();

	//--------------------------
	//--- 发送Fin报文
	//--------------------------
	PktMsgSP first_pkt = Construct4WHSFirstPkt(DEFAULT_SRC_IP, DEFAULT_SRC_PORT
												,DEFAULT_DST_IP, DEFAULT_DST_PORT);
	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_first_pkt));
		
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_; 
	ASSERT_TRUE(fsm_info.conn_state == CONN_CLOSED); 	

	//--------------------------
	//--- 回应 Fin报文和Ack报文
	//--------------------------
	PktMsgSP second_pkt = Construct4WHSSecondPkt(DEFAULT_DST_IP, DEFAULT_DST_PORT
												,DEFAULT_SRC_IP, DEFAULT_SRC_PORT);
	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_second_pkt));
}

/*------------------------------------------------------------------------------
 * 描述: 发送Fin报文， 服务器端发送fin 进入 fin_wait_1 状态
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_4)
{
	AccomplishTcp3WHS();

	//--------------------------
	//--- 发送Fin报文
	//--------------------------
	PktMsgSP first_pkt = Construct4WHSFirstPkt(DEFAULT_DST_IP, DEFAULT_DST_PORT
												,DEFAULT_SRC_IP, DEFAULT_SRC_PORT);
	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_first_pkt));
		
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_;
	ASSERT_TRUE(fsm_info.conn_state == CONN_CLOSED); 	
}


}

