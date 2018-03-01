#ifndef TCP_CONN_ANALYZER_TEST
#define TCP_CONN_ANALYZER_TEST

#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public
#include "tcp_conn_analyzer.hpp"
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
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP& msg_data)
	{
		pkt_num_++;
		return PI_RET_STOP;
	}

private:
	uint32 pkt_num_;
};

class TcpConnAnalyzerTest : public ::testing::Test
{
public:
	typedef TransportLayer<ETHERNET_II, IPV4, TCP> TcpConstructor;
	typedef ApplicationLayer<ETHERNET_II, IPV4, TCP, DUMMY> DummyConstructor;

	TcpConnAnalyzerTest();
	virtual ~TcpConnAnalyzerTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();

protected:

	// 解析报文信息
	bool ParsePktInfo(PktMsgSP& pkt_msg);

	PktMsgSP Construct3WHSFirstPkt();
	PktMsgSP Construct3WHSSecondPkt();
	PktMsgSP Construct3WHSThirdPkt();

	void AccomplishTcp3WHS();

	PktMsgSP Construct4WHSFirstPkt(uint32 src_ip, uint16 src_port,
											uint32 dst_ip, uint16 dst_port);
	PktMsgSP Construct4WHSSecondPkt(uint32 src_ip, uint16 src_port,
												uint32 dst_ip, uint16 dst_port);

protected:
	TcpConnAnalyzer*  tcp_conn_analyzer_;

	boost::shared_ptr<PktProcessorStub> tcp_monitor_successor_;
	boost::shared_ptr<PktProcessorStub> l4_monitor_;
	
};


}

#endif
