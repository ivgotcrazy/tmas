/*#############################################################################
 * �ļ���   : tcp_conn_analyzer_test.cpp
 * ������   : tom_liu
 * ����ʱ�� : 2014��03��5��
 * �ļ����� : TcpConnAnalyzer ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: ����ETH/IP/TCP����Э����Ϣ
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
 * ����: �����������ֵĵ�һ������
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct3WHSFirstPkt()
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
PktMsgSP TcpConnAnalyzerTest::Construct3WHSSecondPkt()
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
PktMsgSP TcpConnAnalyzerTest::Construct3WHSThirdPkt()
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
 * ����: �����Ĵν��ֵĵ�һ������
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct4WHSFirstPkt(uint32 src_ip, uint16 src_port,
															uint32 dst_ip, uint16 dst_port)
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, src_ip, 
		src_port, dst_ip, dst_port);

	// ����TCP�����
	tcp_constructor.SetSeqNum(20001);
	tcp_constructor.SetAckNum(30001);

	tcp_constructor.SetFinFlag(true);
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
 * ����: �����Ĵν��ֵĵڶ�������
 *----------------------------------------------------------------------------*/
PktMsgSP TcpConnAnalyzerTest::Construct4WHSSecondPkt(uint32 src_ip, uint16 src_port,
															uint32 dst_ip, uint16 dst_port)
{
	TcpConstructor tcp_constructor;

	SET_PKT_DEFAULT_PARA(tcp_constructor, src_ip, 
		src_port, dst_ip, dst_port);

	// ����TCP�����
	tcp_constructor.SetSeqNum(30002);
	tcp_constructor.SetAckNum(20002);

	tcp_constructor.SetFinFlag(true);
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
void TcpConnAnalyzerTest::AccomplishTcp3WHS()
{
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_;

	//--------------------------
	//--- �������ֵ�һ������
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
	//--- �������ֵڶ�������
	//--------------------------
	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);

	ASSERT_EQ(TFS_ESTABLISHED, fsm_info.half_conn_fsm[recver].fsm_state);
	

	//--------------------------
	//--- �������ֵ���������
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
 * ����: �������� �ǳ�ʱ
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_1)
{
	AccomplishTcp3WHS();
}

#if 1
/*------------------------------------------------------------------------------
 * ����: �������� ��ʱ
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_2)
{
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_;

	//--------------------------
	//--- �������ֵ�һ������
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
	//--- �������ֵڶ�������
	//--------------------------
	PktMsgSP second_pkt = Construct3WHSSecondPkt();

	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_second_pkt));

	sender = second_pkt->l3_pkt_info.direction;
	recver = (sender + 1) % 2;

	ASSERT_EQ(PKT_DIR_B2S, sender);

	ASSERT_EQ(TFS_ESTABLISHED, fsm_info.half_conn_fsm[recver].fsm_state);

	//��ʱʱ����5��
	boost::this_thread::sleep(boost::posix_time::seconds(5));

	//--------------------------
	//--- �������ֵ���������
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
 * ����: ����Fin���ģ� �ͻ��˷���fin ���� fin_wait_1 ״̬
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_3)
{
	AccomplishTcp3WHS();

	//--------------------------
	//--- ����Fin����
	//--------------------------
	PktMsgSP first_pkt = Construct4WHSFirstPkt(DEFAULT_SRC_IP, DEFAULT_SRC_PORT
												,DEFAULT_DST_IP, DEFAULT_DST_PORT);
	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_first_pkt));
		
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_; 
	ASSERT_TRUE(fsm_info.conn_state == CONN_CLOSED); 	

	//--------------------------
	//--- ��Ӧ Fin���ĺ�Ack����
	//--------------------------
	PktMsgSP second_pkt = Construct4WHSSecondPkt(DEFAULT_DST_IP, DEFAULT_DST_PORT
												,DEFAULT_SRC_IP, DEFAULT_SRC_PORT);
	VoidSP void_second_pkt = VOID_SHARED(second_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_second_pkt));
}

/*------------------------------------------------------------------------------
 * ����: ����Fin���ģ� �������˷���fin ���� fin_wait_1 ״̬
 *----------------------------------------------------------------------------*/
TEST_F(TcpConnAnalyzerTest, Process_Normal_4)
{
	AccomplishTcp3WHS();

	//--------------------------
	//--- ����Fin����
	//--------------------------
	PktMsgSP first_pkt = Construct4WHSFirstPkt(DEFAULT_DST_IP, DEFAULT_DST_PORT
												,DEFAULT_SRC_IP, DEFAULT_SRC_PORT);
	VoidSP void_first_pkt = VOID_SHARED(first_pkt);
	ASSERT_EQ(PI_RET_STOP, tcp_conn_analyzer_->Process(MSG_PKT, void_first_pkt));
		
	TcpConnAnalyzer::TcpFsmInfo& fsm_info = tcp_conn_analyzer_->fsm_info_;
	ASSERT_TRUE(fsm_info.conn_state == CONN_CLOSED); 	
}


}

