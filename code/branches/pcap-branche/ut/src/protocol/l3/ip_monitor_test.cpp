/*#############################################################################
 * �ļ���   : http_parser_test.hpp
 * ������   : tom_liu
 * ����ʱ�� : 2014��02��20��
 * �ļ����� : ����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: ���������Ƿ�Ƭ����
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
 * ����: ����ethЭ���Ip�ı���
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
 * ����: ����IPͷ����ͳ���
 *----------------------------------------------------------------------------*/
TEST_F(IpMonitorTest, IpMonitorTest_Normal_3)
{
	DummyConstructor dummy_constructor(30);

	PktInfoSP info(new PktInfo);
	dummy_constructor.GetPacket((void**)(&(info->pkt.buf)), (size_t*)(&(info->pkt.len)));

	//�޸�Ŀ���ַ, ʹ��У��ͳ���
	IpHdr* hdr = (IpHdr*)(info->pkt.buf + ETH_HEADER_LEN);
	hdr->ip_id = 0x1000;

	ParsePktEthInfo(info);
	VoidSP void_info = VOID_SHARED(info);

	ASSERT_TRUE(ip_monitor_.DoProcess(MSG_PKT, void_info) == PI_RET_STOP);

	delete[] info->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ����IP�汾����ipv4
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
 * ����: ��������Ip��Ƭ����
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
 * ����: ��������Ip��Ƭ���� - �±�����ʼ��ַ ���� �ϱ����ս��ַ
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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //��ʱ������Ӧ����2����Ƭ
	
	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ50
	
	iter++;
	
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ��������Ip��Ƭ���� - �ܳ��ȴ���MTU   ����eth�������1500�����Թ����65000�İ�
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
	//ASSERT_TRUE(!ip_monitor_.ip_frags_.empty()); //�˰��ᱻ����

	ip_monitor_.ip_frags_.clear();

	delete[] info1->pkt.buf;

}

/*------------------------------------------------------------------------------
 * ����: ��������Ip��Ƭ���� - �ظ��յ��ױ���
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ��Ƭ�����ݳ���ӦΪ50 

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ��������Ip��Ƭ���� - �ظ��յ�β����
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ��Ƭ�����ݳ���ӦΪ60 

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
	
}


/*------------------------------------------------------------------------------
 * ����: ���������ظ���Ip��Ƭ���� - �±��ĵ��ս��ַ�������ϱ��ĵ���ʼ��ַ
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //��ʱ������Ӧ����2����Ƭ

	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 60);  //��ʱ��Ƭ�����ݳ���ӦΪ60

	iter++;
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ��Ƭ�����ݳ���ӦΪ50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ���������ظ���Ip��Ƭ���� - �±��İ����ϱ���
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ
	
	auto iter = pkt_iter->entry.frags.begin();

	LOG(INFO) << "DATA SIZE: " << iter->data.size();
	ASSERT_TRUE(iter->data.size() == 60);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ60

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ���������ظ���Ip��Ƭ���� - ԭ���İ����±���
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ
	
	auto iter = pkt_iter->entry.frags.begin();

	LOG(INFO) << "DATA SIZE: " << iter->data.size();
	ASSERT_TRUE(iter->data.size() == 80);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;

}


/*------------------------------------------------------------------------------
 * ����: ���������ظ���Ip��Ƭ���� - ԭ�������ཻ�±���
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //��ʱ������Ӧ����2����Ƭ

	auto iter = pkt_iter->entry.frags.begin();

	LOG(INFO) << "DATA SIZE: " << iter->data.size();
	ASSERT_TRUE(iter->data.size() == 10);  //��ʱ��һ��Ƭ����Ϊ10

	iter++;
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ��һ��Ƭ����Ϊ50

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;
}

/*------------------------------------------------------------------------------
 * ����: ���������ظ���Ip��Ƭ���� - ԭ�������ཻ�±���
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

	ASSERT_TRUE(pkt_iter1->entry.frags.size() == 1);  //��ʱ������Ӧ����1����Ƭ

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

	ASSERT_TRUE(pkt_iter->entry.frags.size() == 2);  //��ʱ������Ӧ����2����Ƭ
	
	auto iter = pkt_iter->entry.frags.begin();
	ASSERT_TRUE(iter->data.size() == 50);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ50

	iter++;
	ASSERT_TRUE(iter->offset == 50);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ50
	ASSERT_TRUE(iter->data.size() == 30);  //��ʱ����ķ�Ƭ�����ݳ���ӦΪ30

	ip_monitor_.ip_frags_.clear();

	delete[] info2->pkt.buf;

}


/*------------------------------------------------------------------------------
 * ����: ���������ظ���Ip��Ƭ���� - ��ʱ����Ч
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



