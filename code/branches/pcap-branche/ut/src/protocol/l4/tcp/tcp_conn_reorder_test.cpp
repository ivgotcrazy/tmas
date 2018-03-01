/*#############################################################################
 * 文件名   : tcp_conn_reorder_test.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月22日
 * 文件描述 : HttpParserTest实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_conn_reorder_test.hpp"

#include "pkt_parser.hpp"
#include "tmas_config_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

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

TcpConnReorderTest::TcpConnReorderTest() 
	: tcp_monitor_successor_(new PktProcessorStub())
	, tcp_reorder_successor_(new PktProcessorStub())
	, tcp_conn_reorder_(nullptr)
{
	TmasConfigParser::GetInstance().Init();
}

TcpConnReorderTest::~TcpConnReorderTest()
{
}

void TcpConnReorderTest::SetUp()
{
	tcp_conn_reorder_ = new TcpConnReorder(new TcpMonitor(tcp_monitor_successor_));

	tcp_conn_reorder_->set_successor(tcp_reorder_successor_);
}

void TcpConnReorderTest::TearDown()
{
	delete tcp_conn_reorder_;
}

/*------------------------------------------------------------------------------
 * 描述: 构建三次握手的第一个报文
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnReorderTest::Construct3WHSFirstPkt()
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
PktMsgSP TcpConnReorderTest::Construct3WHSSecondPkt()
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
PktMsgSP TcpConnReorderTest::Construct3WHSThirdPkt()
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
 * 描述: 完成TCP三次握手
 *----------------------------------------------------------------------------*/
void TcpConnReorderTest::AccomplishTcp3WHS()
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- 三次握手第一个报文
	//--------------------------

	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_first_pkt));

	uint8 sender = first_pkt->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_TRUE(conn_info.started);
	
	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(1, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(10001, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//--------------------------
	//--- 三次握手第二个报文
	//--------------------------

	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(1, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(20001, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(1, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(10001, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(20002, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].next_seq_num);

	//--------------------------
	//--- 三次握手第三个报文
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	sender = third_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(1, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(20001, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(20002, conn_info.half_conn[recver].next_seq_num);

	delete[] first_pkt->pkt.buf;
	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析ETH/IP/TCP三层协议信息
 *----------------------------------------------------------------------------*/
bool TcpConnReorderTest::ParsePktInfo(PktMsgSP& pkt_msg)
{
	if (!ParsePktEthInfo(pkt_msg)) return false;

	if (!ParsePktIpInfo(true, pkt_msg)) return false;

	if (!ParsePktTcpInfo(pkt_msg)) return false;
	
	return true;
}

//==============================================================================
// Test case implementation
//==============================================================================

/*------------------------------------------------------------------------------
 * 描述: 三次握手 -> 数据传输，顺序
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Normal_1)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//---------------------------------------------------------------
	//--- 三次握手
	//---------------------------------------------------------------

	AccomplishTcp3WHS();

	ASSERT_EQ(3, tcp_reorder_successor_->GetPktNum());

	//---------------------------------------------------------------
	//--- 数据请求报文1
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request_1 = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_1, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request_1.SetSeqNum(10002);
	tcp_request_1.SetAckNum(20002);
	tcp_request_1.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request_1(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request_1->pkt.buf)),
		                          (size_t*)(&(pkt_request_1->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_1));

	VoidSP void_request_1 = VOID_SHARED(pkt_request_1);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_1));

	// 校验

	uint8 sender = pkt_request_1->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(3, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(10130, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(1, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(20001, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(10130, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(20002, conn_info.half_conn[recver].next_seq_num);

	//---------------------------------------------------------------
	//--- 数据响应报文1
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response_1 = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response_1, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// 设置TCP层参数
	tcp_response_1.SetSeqNum(20002);
	tcp_response_1.SetAckNum(10130);

	tcp_response_1.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_response_1(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response_1->pkt.buf)),
								  (size_t*)(&(pkt_response_1->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response_1));

	VoidSP void_response_1 = VOID_SHARED(pkt_response_1);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response_1));

	// 校验

	sender = pkt_response_1->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(10130, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(10130, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(20130, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(3, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(20002, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(20130, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(10130, conn_info.half_conn[recver].next_seq_num);

	delete[] pkt_request_1->pkt.buf;
	delete[] pkt_response_1->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 乱序
 *       1. 三次握手的第三个报文
 *       2. 三次握手的第二个报文
 *       3. 三次握手的第一个报文
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_1)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- 三次握手第三个报文
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	uint8 sender = third_pkt->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(1, conn_info.half_conn[sender].cached_pkt.size());

	ASSERT_EQ(0, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//--------------------------
	//--- 三次握手第二个报文
	//--------------------------

	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(1, conn_info.half_conn[sender].cached_pkt.size());

	ASSERT_EQ(0, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(1, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//--------------------------
	//--- 三次握手第一个报文
	//--------------------------

	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_first_pkt));

	sender = first_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(1, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(20001, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(10002, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(20002, conn_info.half_conn[recver].next_seq_num);

	delete[] first_pkt->pkt.buf;
	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 乱序
 *       1. 三次握手的第三个报文
 *       2. 数据响应报文
 *       3. 数据请求报文
 *       4. 三次握手的第二个报文
 *       5. 三次握手的第一个报文
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_2)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- 三次握手第三个报文
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	uint8 sender = third_pkt->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(1, conn_info.half_conn[sender].cached_pkt.size());

	ASSERT_EQ(0, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//---------------------------------------------------------------
	//--- 数据响应报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// 设置TCP层参数
	tcp_response.SetSeqNum(20002);
	tcp_response.SetAckNum(10130);

	tcp_response.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_response(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response->pkt.buf)),
		(size_t*)(&(pkt_response->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response));

	VoidSP void_response = VOID_SHARED(pkt_response);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response));

	// 校验

	sender = pkt_response->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(1, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(1, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//---------------------------------------------------------------
	//--- 数据请求报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request.SetSeqNum(10002);
	tcp_request.SetAckNum(20002);
	tcp_request.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request->pkt.buf)),
		(size_t*)(&(pkt_request->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request));

	VoidSP void_request = VOID_SHARED(pkt_request);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request));

	// 校验

	sender = pkt_request->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(2, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(1, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//--------------------------
	//--- 三次握手第二个报文
	//--------------------------

	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(2, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(2, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(0, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(0, conn_info.half_conn[recver].next_seq_num);

	//--------------------------
	//--- 三次握手第一个报文
	//--------------------------

	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_first_pkt));

	sender = first_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(0, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(3, conn_info.half_conn[sender].send_pkt_num);
	ASSERT_EQ(20002, conn_info.half_conn[sender].sent_ack_num);
	ASSERT_EQ(10002, conn_info.half_conn[sender].sent_seq_num);
	ASSERT_EQ(20130, conn_info.half_conn[sender].next_ack_num);
	ASSERT_EQ(10130, conn_info.half_conn[sender].next_seq_num);

	ASSERT_EQ(0, conn_info.half_conn[recver].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[recver].send_pkt_num);
	ASSERT_EQ(10130, conn_info.half_conn[recver].sent_ack_num);
	ASSERT_EQ(20002, conn_info.half_conn[recver].sent_seq_num);
	ASSERT_EQ(10130, conn_info.half_conn[recver].next_ack_num);
	ASSERT_EQ(20130, conn_info.half_conn[recver].next_seq_num);

	delete[] first_pkt->pkt.buf;
	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
	delete[] pkt_request->pkt.buf;
	delete[] pkt_response->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 缓存超过3个报文还未开始进行握手
 *       1. 三次握手的第三个报文
 *       2. 数据响应报文
 *       3. 数据请求报文
 *       4. 三次握手的第二个报文
 *       5. 数据请求报文
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_3)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- 报文1: 三次握手第三个报文
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	//---------------------------------------------------------------
	//--- 报文2: 数据响应报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// 设置TCP层参数
	tcp_response.SetSeqNum(20002);
	tcp_response.SetAckNum(10130);
	tcp_response.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_response(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response->pkt.buf)),
		(size_t*)(&(pkt_response->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response));

	VoidSP void_response = VOID_SHARED(pkt_response);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response));

	//---------------------------------------------------------------
	//--- 报文3: 数据请求报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request.SetSeqNum(10002);
	tcp_request.SetAckNum(20002);
	tcp_request.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request->pkt.buf)),
		(size_t*)(&(pkt_request->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request));

	VoidSP void_request = VOID_SHARED(pkt_request);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request));

	//--------------------------
	//--- 报文4: 三次握手第二个报文
	//--------------------------

	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_second_pkt));

	//---------------------------------------------------------------
	//--- 报文5: 数据请求报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_3(128);
	TcpConstructor& tcp_request_2 = *(dummy_constructor_3.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_2, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request_2.SetSeqNum(10130);
	tcp_request_2.SetAckNum(20130);
	tcp_request_2.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request_2(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request_2->pkt.buf)),
		(size_t*)(&(pkt_request_2->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_2));

	VoidSP void_request_2 = VOID_SHARED(pkt_request_2);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_2));

	// 校验

	uint8 sender = pkt_request_2->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(2, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[recver].cached_pkt.size());

	// TODO: 这里通过观察覆盖率统计确认程序行为的，如果能够打桩最好

	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
	delete[] pkt_request->pkt.buf;
	delete[] pkt_response->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 开始握手后，缓存太多报文
 *       1. 三次握手的第一个报文
 *       2. 三次握手的第三个报文
 *       3. 数据响应报文
 *       4. 数据请求报文
 *       5. 数据响应报文
 *       6. 数据请求报文
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_4)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- 报文1: 三次握手第一个报文
	//--------------------------

	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_first_pkt));

	//--------------------------
	//--- 报文2: 三次握手第三个报文
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	//---------------------------------------------------------------
	//--- 报文3: 数据响应报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// 设置TCP层参数
	tcp_response.SetSeqNum(20002);
	tcp_response.SetAckNum(10130);
	tcp_response.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_response(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response->pkt.buf)),
		(size_t*)(&(pkt_response->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response));

	VoidSP void_response = VOID_SHARED(pkt_response);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response));

	//---------------------------------------------------------------
	//--- 报文4: 数据请求报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request.SetSeqNum(10002);
	tcp_request.SetAckNum(20002);
	tcp_request.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request->pkt.buf)),
		(size_t*)(&(pkt_request->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request));

	VoidSP void_request = VOID_SHARED(pkt_request);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request));

	//---------------------------------------------------------------
	//--- 报文5: 数据响应报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_4(128);
	TcpConstructor& tcp_response_2 = *(dummy_constructor_4.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response_2, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// 设置TCP层参数
	tcp_response_2.SetSeqNum(20130);
	tcp_response_2.SetAckNum(10258);
	tcp_response_2.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_response_2(new PktMsg());
	dummy_constructor_4.GetPacket((void**)(&(pkt_response_2->pkt.buf)),
		(size_t*)(&(pkt_response_2->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response_2));

	VoidSP void_response_2 = VOID_SHARED(pkt_response_2);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response_2));


	//---------------------------------------------------------------
	//--- 报文6: 数据请求报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_5(128);
	TcpConstructor& tcp_request_3 = *(dummy_constructor_5.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_3, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request_3.SetSeqNum(10258);
	tcp_request_3.SetAckNum(20258);
	tcp_request_3.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request_3(new PktMsg());
	dummy_constructor_5.GetPacket((void**)(&(pkt_request_3->pkt.buf)),
							      (size_t*)(&(pkt_request_3->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_3));

	VoidSP void_request_3 = VOID_SHARED(pkt_request_3);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_3));


	//---------------------------------------------------------------
	//--- 报文7: 数据请求报文
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_3(128);
	TcpConstructor& tcp_request_2 = *(dummy_constructor_3.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_2, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// 设置TCP层参数
	tcp_request_2.SetSeqNum(10130);
	tcp_request_2.SetAckNum(20130);
	tcp_request_2.SetAckFlag(true);

	// 处理报文
	PktMsgSP pkt_request_2(new PktMsg());
	dummy_constructor_3.GetPacket((void**)(&(pkt_request_2->pkt.buf)),
								  (size_t*)(&(pkt_request_2->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_2));

	VoidSP void_request_2 = VOID_SHARED(pkt_request_2);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_2));

	// 校验

	uint8 sender = pkt_request_2->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_TRUE(conn_info.started);

	ASSERT_EQ(4, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[recver].cached_pkt.size());

	delete[] third_pkt->pkt.buf;
	delete[] pkt_request->pkt.buf;
	delete[] pkt_response->pkt.buf;
	delete[] pkt_request_2->pkt.buf;
	delete[] pkt_response_2->pkt.buf;
}

} // namespace BroadInter