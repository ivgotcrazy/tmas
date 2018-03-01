/*#############################################################################
 * 文件名   : http_parser_test.hpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年02月20日
 * 文件描述 : 声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <iostream>
#include <fstream>

#include "ip_monitor_test.hpp"

#include "eth_monitor.hpp"
#include "pkt_parser.hpp"

#include "eth_layer.hpp"
#include "ip_layer.hpp"
#include "transport_layer.hpp"

namespace BroadInter
{
//==============================================================================
// Test class implementation
//==============================================================================

IpMonitorTest::IpMonitorTest()
{
	ip_monitor_.Init();
}

IpMonitorTest::~IpMonitorTest()
{
}

void IpMonitorTest::SetUp()
{

}

void IpMonitorTest::TearDown()
{

}

//==============================================================================
// Test case implementation
//==============================================================================

/*------------------------------------------------------------------------------
 * 描述: 解析正常非分片报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_1)
{
	DummyConstructor dummy_constructor(30);

	PktInfoSP info(new PktInfo);
	dummy_constructor.GetPacket((void**)(&(info->pkt.buf)), (size_t*)(&(info->pkt.len)));
	
	ParsePktEthInfo(info);
	VoidSP void_info = VOID_SHARED(info);
	
	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info) == PI_CHAIN_CONTINUE);
	
	delete[] info->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析eth协议非Ip的报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_2)
{
	DummyConstructor dummy_constructor(30);

	EthConstructor& eth_constructor = *(dummy_constructor.GetEthLayer());
	eth_constructor.SetUpperLayerProtocol(0x8400);

	PktInfoSP info(new PktInfo);
	dummy_constructor.GetPacket((void**)(&(info->pkt.buf)), (size_t*)(&(info->pkt.len)));

	ParsePktEthInfo(info);
	VoidSP void_info = VOID_SHARED(info);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info) == PI_RET_STOP);

	delete[] info->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析IP头检验和出错
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_3)
{
	DummyConstructor dummy_constructor(30);

	PktInfoSP info(new PktInfo);
	dummy_constructor.GetPacket((void**)(&(info->pkt.buf)), (size_t*)(&(info->pkt.len)));

	//修改目标地址, 使得校验和出错
	IpHdr* hdr = (IpHdr*)(info->pkt.buf + ETH_HEADER_LEN);
	hdr->ip_id = 0x1000;

	ParsePktEthInfo(info);
	VoidSP void_info = VOID_SHARED(info);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info) == PI_RET_STOP);

	delete[] info->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析IP版本不是ipv4
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_4)
{
	DummyConstructor dummy_constructor(30);

	IpConstructor& ip_constructor = *(dummy_constructor.GetIpLayer());
	ip_constructor.SetIpVersion(6);

	PktInfoSP info(new PktInfo);
	dummy_constructor.GetPacket((void**)(&(info->pkt.buf)), (size_t*)(&(info->pkt.len)));

	ParsePktEthInfo(info);
	VoidSP void_info = VOID_SHARED(info);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info) == PI_RET_STOP);

	delete[] info->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理Ip分片报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_5)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(!ip_monitor_.ip_frags_.empty());

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(30);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpOffSet(50);
	ip_constructor2.SetIpId(0x20);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.empty());

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理Ip分片报文 - 新报文起始地址 等于 老报文终结地址
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_6)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(!ip_monitor_.ip_frags_.empty());

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(30);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpOffSet(50);
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //此时容器中应该有2个分片
	
	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //此时后面的分片的数据长度应为50
	
	iter++;
	
	ASSERT_TRUE(iter->data.size() == 50);  //此时后面的分片的数据长度应为50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理Ip分片报文 - 总长度大于MTU   由于eth最大数据1500，难以构造出65000的包
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_7)
{
	DummyConstructor dummy_constructor1(50);  //65000

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);
	ip_constructor1.SetIpOffSet(50);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	//ASSERT_TRUE(!ip_monitor_.ip_frags_.empty()); //此包会被丢弃

	ip_monitor_.ip_frags_.clear();

	delete[] info1->pkt.buf;

}

/*------------------------------------------------------------------------------
 * 描述: 解析处理Ip分片报文 - 重复收到首报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_8)
{	
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////
	
	DummyConstructor dummy_constructor2(40);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //此时容器中应该有1个分片

	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //此时分片的数据长度应为50 

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理Ip分片报文 - 重复收到尾报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_9)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpOffSet(30);
	ip_constructor1.SetIpId(0x20);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(40);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpOffSet(40);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //此时容器中应该有1个分片

	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //此时分片的数据长度应为60 

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
	
}


/*------------------------------------------------------------------------------
 * 描述: 解析处理重复的Ip分片报文 - 新报文的终结地址不大于老报文的起始地址
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_10)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpOffSet(100);
	ip_constructor1.SetIpId(0x20);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(40);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //此时容器中应该有2个分片

	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 60);  //此时分片的数据长度应为60

	iter++;
	ASSERT_TRUE(iter->data.size() == 50);  //此时分片的数据长度应为50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理重复的Ip分片报文 - 新报文包含老报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_11)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpOffSet(10);
	ip_constructor1.SetIpId(0x20);
	ip_constructor1.SetIpMfFlag(true);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(40);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);
	ip_constructor2.SetIpOffSet(5);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //此时容器中应该有1个分片
	
	auto iter = pkt_iter->entry.frags.begin();

	LOG(INFO) << "DATA SIZE: " << iter->data.size();
	ASSERT_TRUE(iter->data.size() == 60);  //此时后面的分片的数据长度应为60

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理重复的Ip分片报文 - 原报文包含新报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_12)
{
	DummyConstructor dummy_constructor1(60);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpId(0x20);
	ip_constructor1.SetIpMfFlag(true);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(20);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);
	ip_constructor2.SetIpOffSet(10);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //此时容器中应该有1个分片
	
	auto iter = pkt_iter->entry.frags.begin();

	LOG(INFO) << "DATA SIZE: " << iter->data.size();
	ASSERT_TRUE(iter->data.size() == 80);  //此时后面的分片的数据长度应为50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;

}


/*------------------------------------------------------------------------------
 * 描述: 解析处理重复的Ip分片报文 - 原报文右相交新报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_13)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());
	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(30);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);
	ip_constructor2.SetIpOffSet(10);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //此时容器中应该有2个分片

	auto iter = pkt_iter->entry.frags.begin();

	LOG(INFO) << "DATA SIZE: " << iter->data.size();
	ASSERT_TRUE(iter->data.size() == 10);  //此时第一分片长度为10

	iter++;
	ASSERT_TRUE(iter->data.size() == 50);  //此时第一分片长度为50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * 描述: 解析处理重复的Ip分片报文 - 原报文左相交新报文
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_14)
{
	DummyConstructor dummy_constructor1(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor1.GetIpLayer());

	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);
	ip_constructor1.SetIpOffSet(30);

	PktInfoSP info1(new PktInfo);
	dummy_constructor1.GetPacket((void**)(&(info1->pkt.buf)), (size_t*)(&(info1->pkt.len)));

	ParsePktEthInfo(info1);
	VoidSP void_info1 = VOID_SHARED(info1);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info1) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view1 = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter1 = frag_id_view1.begin();

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //此时容器中应该有1个分片

	delete[] info1->pkt.buf;

	//////////////////////////////////////////////////////////////////////////////

	DummyConstructor dummy_constructor2(30);

	IpConstructor& ip_constructor2 = *(dummy_constructor2.GetIpLayer());
	ip_constructor2.SetIpId(0x20);
	ip_constructor2.SetIpMfFlag(true);

	PktInfoSP info2(new PktInfo);
	dummy_constructor2.GetPacket((void**)(&(info2->pkt.buf)), (size_t*)(&(info2->pkt.len)));

	ParsePktEthInfo(info2);
	VoidSP void_info2 = VOID_SHARED(info2);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info2) == PI_RET_STOP);
	ASSERT_TRUE(ip_monitor_.ip_frags_.size() == 1);

	IpMonitor::FragIdView& frag_id_view = ip_monitor_.ip_frags_.get<0>();
	IpMonitor::FragPkMapIter pkt_iter = frag_id_view.begin();

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //此时容器中应该有2个分片
	
	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //此时后面的分片的数据长度应为50

	iter++;
	ASSERT_TRUE(iter->offset == 50);  //此时后面的分片的数据长度应为50
	ASSERT_TRUE(iter->data.size() == 30);  //此时后面的分片的数据长度应为30

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;

}


/*------------------------------------------------------------------------------
 * 描述: 解析处理重复的Ip分片报文 - 定时器生效
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_15)
{
	DummyConstructor dummy_constructor(30);

	IpConstructor& ip_constructor1 = *(dummy_constructor.GetIpLayer());

	ip_constructor1.SetIpMfFlag(true);
	ip_constructor1.SetIpId(0x20);
	ip_constructor1.SetIpOffSet(30);
	
	PktInfoSP info(new PktInfo);
	dummy_constructor.GetPacket((void**)(&(info->pkt.buf)), (size_t*)(&(info->pkt.len)));
	
	ParsePktEthInfo(info);
	VoidSP void_info = VOID_SHARED(info);
	
	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info) == PI_RET_STOP);
	ASSERT_TRUE(!ip_monitor_.ip_frags_.empty());

	boost::this_thread::sleep(boost::posix_time::seconds(5));

	ASSERT_TRUE(ip_monitor_.ip_frags_.empty());

	ip_monitor_.ip_frags_.clear();

	delete[] info->pkt.buf;
}


}



