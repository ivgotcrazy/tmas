/*#############################################################################
 * �ļ���   : tcp_conn_reorder_test.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��22��
 * �ļ����� : HttpParserTestʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: �����������ֵĵ�һ������
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnReorderTest::Construct3WHSFirstPkt()
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP����
	tcp_constructor.SetSeqNum(10001);
	tcp_constructor.SetAckNum(0);

	tcp_constructor.SetSynFlag(true);

	// ��ȡPktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// ��������
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * ����: �����������ֵĵڶ�������
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnReorderTest::Construct3WHSSecondPkt()
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// ����TCP�����
	tcp_constructor.SetSeqNum(20001);
	tcp_constructor.SetAckNum(10002);

	tcp_constructor.SetSynFlag(true);
	tcp_constructor.SetAckFlag(true);

	// ��ȡPktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// ��������
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * ����: �����������ֵĵ���������
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnReorderTest::Construct3WHSThirdPkt()
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_constructor.SetSeqNum(10002);
	tcp_constructor.SetAckNum(20002);

	tcp_constructor.SetAckFlag(true);

	// ��ȡPktMsg
	PktMsgSP pkt_msg(new PktMsg);
	tcp_constructor.GetPacket((void**)(&(pkt_msg->pkt.buf)), 
		                      (size_t*)(&(pkt_msg->pkt.len)));

	// ��������
	TMAS_ASSERT(ParsePktInfo(pkt_msg));

	return pkt_msg;
}

/*------------------------------------------------------------------------------
 * ����: ���TCP��������
 *----------------------------------------------------------------------------*/
void TcpConnReorderTest::AccomplishTcp3WHS()
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- �������ֵ�һ������
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
	//--- �������ֵڶ�������
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
	//--- �������ֵ���������
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
 * ����: ����ETH/IP/TCP����Э����Ϣ
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
 * ����: �������� -> ���ݴ��䣬˳��
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Normal_1)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//---------------------------------------------------------------
	//--- ��������
	//---------------------------------------------------------------

	AccomplishTcp3WHS();

	ASSERT_EQ(3, tcp_reorder_successor_->GetPktNum());

	//---------------------------------------------------------------
	//--- ����������1
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request_1 = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_1, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request_1.SetSeqNum(10002);
	tcp_request_1.SetAckNum(20002);
	tcp_request_1.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request_1(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request_1->pkt.buf)),
		                          (size_t*)(&(pkt_request_1->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_1));

	VoidSP void_request_1 = VOID_SHARED(pkt_request_1);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_1));

	// У��

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
	//--- ������Ӧ����1
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response_1 = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response_1, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// ����TCP�����
	tcp_response_1.SetSeqNum(20002);
	tcp_response_1.SetAckNum(10130);

	tcp_response_1.SetAckFlag(true);

	// ������
	PktMsgSP pkt_response_1(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response_1->pkt.buf)),
								  (size_t*)(&(pkt_response_1->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response_1));

	VoidSP void_response_1 = VOID_SHARED(pkt_response_1);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response_1));

	// У��

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
 * ����: ����
 *       1. �������ֵĵ���������
 *       2. �������ֵĵڶ�������
 *       3. �������ֵĵ�һ������
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_1)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- �������ֵ���������
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
	//--- �������ֵڶ�������
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
	//--- �������ֵ�һ������
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
 * ����: ����
 *       1. �������ֵĵ���������
 *       2. ������Ӧ����
 *       3. ����������
 *       4. �������ֵĵڶ�������
 *       5. �������ֵĵ�һ������
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_2)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- �������ֵ���������
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
	//--- ������Ӧ����
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// ����TCP�����
	tcp_response.SetSeqNum(20002);
	tcp_response.SetAckNum(10130);

	tcp_response.SetAckFlag(true);

	// ������
	PktMsgSP pkt_response(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response->pkt.buf)),
		(size_t*)(&(pkt_response->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response));

	VoidSP void_response = VOID_SHARED(pkt_response);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response));

	// У��

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
	//--- ����������
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request.SetSeqNum(10002);
	tcp_request.SetAckNum(20002);
	tcp_request.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request->pkt.buf)),
		(size_t*)(&(pkt_request->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request));

	VoidSP void_request = VOID_SHARED(pkt_request);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request));

	// У��

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
	//--- �������ֵڶ�������
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
	//--- �������ֵ�һ������
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
 * ����: ���泬��3�����Ļ�δ��ʼ��������
 *       1. �������ֵĵ���������
 *       2. ������Ӧ����
 *       3. ����������
 *       4. �������ֵĵڶ�������
 *       5. ����������
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_3)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- ����1: �������ֵ���������
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	//---------------------------------------------------------------
	//--- ����2: ������Ӧ����
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// ����TCP�����
	tcp_response.SetSeqNum(20002);
	tcp_response.SetAckNum(10130);
	tcp_response.SetAckFlag(true);

	// ������
	PktMsgSP pkt_response(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response->pkt.buf)),
		(size_t*)(&(pkt_response->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response));

	VoidSP void_response = VOID_SHARED(pkt_response);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response));

	//---------------------------------------------------------------
	//--- ����3: ����������
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request.SetSeqNum(10002);
	tcp_request.SetAckNum(20002);
	tcp_request.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request->pkt.buf)),
		(size_t*)(&(pkt_request->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request));

	VoidSP void_request = VOID_SHARED(pkt_request);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request));

	//--------------------------
	//--- ����4: �������ֵڶ�������
	//--------------------------

	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_second_pkt));

	//---------------------------------------------------------------
	//--- ����5: ����������
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_3(128);
	TcpConstructor& tcp_request_2 = *(dummy_constructor_3.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_2, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request_2.SetSeqNum(10130);
	tcp_request_2.SetAckNum(20130);
	tcp_request_2.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request_2(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request_2->pkt.buf)),
		(size_t*)(&(pkt_request_2->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_2));

	VoidSP void_request_2 = VOID_SHARED(pkt_request_2);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_2));

	// У��

	uint8 sender = pkt_request_2->l3_pkt_info.direction;
	uint8 recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_S2B, sender);
	ASSERT_FALSE(conn_info.started);

	ASSERT_EQ(2, conn_info.half_conn[sender].cached_pkt.size());
	ASSERT_EQ(2, conn_info.half_conn[recver].cached_pkt.size());

	// TODO: ����ͨ���۲츲����ͳ��ȷ�ϳ�����Ϊ�ģ�����ܹ���׮���

	delete[] second_pkt->pkt.buf;
	delete[] third_pkt->pkt.buf;
	delete[] pkt_request->pkt.buf;
	delete[] pkt_response->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ��ʼ���ֺ󣬻���̫�౨��
 *       1. �������ֵĵ�һ������
 *       2. �������ֵĵ���������
 *       3. ������Ӧ����
 *       4. ����������
 *       5. ������Ӧ����
 *       6. ����������
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnReorderTest, Process_Disorder_4)
{
	TcpConnReorder::TcpConnInfo& conn_info = tcp_conn_reorder_->conn_info_;

	//--------------------------
	//--- ����1: �������ֵ�һ������
	//--------------------------

	PktMsgSP first_pkt = Construct3WHSFirstPkt();

	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_first_pkt));

	//--------------------------
	//--- ����2: �������ֵ���������
	//--------------------------

	PktMsgSP third_pkt = Construct3WHSThirdPkt();

	VoidSP void_third_pkt = VOID_SHARED(third_pkt);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_third_pkt));

	//---------------------------------------------------------------
	//--- ����3: ������Ӧ����
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_2(128);
	TcpConstructor& tcp_response = *(dummy_constructor_2.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// ����TCP�����
	tcp_response.SetSeqNum(20002);
	tcp_response.SetAckNum(10130);
	tcp_response.SetAckFlag(true);

	// ������
	PktMsgSP pkt_response(new PktMsg());
	dummy_constructor_2.GetPacket((void**)(&(pkt_response->pkt.buf)),
		(size_t*)(&(pkt_response->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response));

	VoidSP void_response = VOID_SHARED(pkt_response);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response));

	//---------------------------------------------------------------
	//--- ����4: ����������
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_1(128);
	TcpConstructor& tcp_request = *(dummy_constructor_1.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request.SetSeqNum(10002);
	tcp_request.SetAckNum(20002);
	tcp_request.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request(new PktMsg());
	dummy_constructor_1.GetPacket((void**)(&(pkt_request->pkt.buf)),
		(size_t*)(&(pkt_request->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request));

	VoidSP void_request = VOID_SHARED(pkt_request);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request));

	//---------------------------------------------------------------
	//--- ����5: ������Ӧ����
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_4(128);
	TcpConstructor& tcp_response_2 = *(dummy_constructor_4.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_response_2, DEFAULT_DST_IP, 
		DEFAULT_DST_PORT, DEFAULT_SRC_IP, DEFAULT_SRC_PORT);

	// ����TCP�����
	tcp_response_2.SetSeqNum(20130);
	tcp_response_2.SetAckNum(10258);
	tcp_response_2.SetAckFlag(true);

	// ������
	PktMsgSP pkt_response_2(new PktMsg());
	dummy_constructor_4.GetPacket((void**)(&(pkt_response_2->pkt.buf)),
		(size_t*)(&(pkt_response_2->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_response_2));

	VoidSP void_response_2 = VOID_SHARED(pkt_response_2);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_response_2));


	//---------------------------------------------------------------
	//--- ����6: ����������
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_5(128);
	TcpConstructor& tcp_request_3 = *(dummy_constructor_5.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_3, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request_3.SetSeqNum(10258);
	tcp_request_3.SetAckNum(20258);
	tcp_request_3.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request_3(new PktMsg());
	dummy_constructor_5.GetPacket((void**)(&(pkt_request_3->pkt.buf)),
							      (size_t*)(&(pkt_request_3->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_3));

	VoidSP void_request_3 = VOID_SHARED(pkt_request_3);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_3));


	//---------------------------------------------------------------
	//--- ����7: ����������
	//---------------------------------------------------------------

	DummyConstructor dummy_constructor_3(128);
	TcpConstructor& tcp_request_2 = *(dummy_constructor_3.GetTransportLayer());

	SET_PKT_DEFAULT_PARA(tcp_request_2, DEFAULT_SRC_IP, 
		DEFAULT_SRC_PORT, DEFAULT_DST_IP, DEFAULT_DST_PORT);

	// ����TCP�����
	tcp_request_2.SetSeqNum(10130);
	tcp_request_2.SetAckNum(20130);
	tcp_request_2.SetAckFlag(true);

	// ������
	PktMsgSP pkt_request_2(new PktMsg());
	dummy_constructor_3.GetPacket((void**)(&(pkt_request_2->pkt.buf)),
								  (size_t*)(&(pkt_request_2->pkt.len)));

	ASSERT_TRUE(ParsePktInfo(pkt_request_2));

	VoidSP void_request_2 = VOID_SHARED(pkt_request_2);
	ASSERT_EQ(PI_HAS_PROCESSED, tcp_conn_reorder_->Process(MSG_PKT, void_request_2));

	// У��

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