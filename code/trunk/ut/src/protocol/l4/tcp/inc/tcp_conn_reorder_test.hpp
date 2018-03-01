/*#############################################################################
 * �ļ���   : tcp_conn_reorder_test.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��22��
 * �ļ����� : HttpParserTest����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_CONN_REORDER_TEST
#define BROADINTER_TCP_CONN_REORDER_TEST

#include <gtest/gtest.h>

#define private public
#define protected public
#include "tcp_conn_reorder.hpp"
#undef private
#undef protected

#define private public
#define protected public
#include "tcp_monitor.hpp"
#undef private
#undef protected


#include "message.hpp"
#include "pkt_processor.hpp"
#include "eth_layer.hpp"
#include "ip_layer.hpp"
#include "transport_layer.hpp"
#include "application_layer.hpp"

namespace BroadInter
{

class PktProcessorStub : public PktProcessor
{
public:
	PktProcessorStub() : pkt_num_(0) {}

	uint32 GetPktNum() const { return pkt_num_; }

private:
	virtual ProcInfo DoProcess(MsgType msg_type, void* msg_data)
	{
		pkt_num_++;
		return PI_HAS_PROCESSED;
	}

private:
	uint32 pkt_num_;
};



class TcpConnReorderTest : public ::testing::Test
{
protected:

	typedef TransportLayer<ETHERNET_II, IPV4, TCP> TcpConstructor;
	typedef ApplicationLayer<ETHERNET_II, IPV4, TCP, DUMMY> DummyConstructor;

	TcpConnReorderTest();
	virtual ~TcpConnReorderTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();

	// Objects declared here can be used by all tests in the test case.

	// ����������Ϣ
	bool ParsePktInfo(PktMsg* pkt_msg);

	// �����������ֱ���
	PktMsgSP Construct3WHSFirstPkt();
	PktMsgSP Construct3WHSSecondPkt();
	PktMsgSP Construct3WHSThirdPkt();

	// �����������
	void AccomplishTcp3WHS();

	boost::shared_ptr<PktProcessorStub> tcp_monitor_successor_;
	boost::shared_ptr<PktProcessorStub> tcp_reorder_successor_;

	TcpConnReorder* tcp_conn_reorder_;
};

}

#endif