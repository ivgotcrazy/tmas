/*#############################################################################
 * �ļ���   : ipv4_frag_reassembler_test.hpp
 * ������   : teck_zhou
 * ����ʱ�� : 2014��04��02��
 * �ļ����� : Ipv4FragReassemblerTest����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IPV4_FRAG_REASSEMBLER_TEST
#define BROADINTER_IPV4_FRAG_REASSEMBLER_TEST

#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public
#include "ipv4_frag_reassembler.hpp"
#undef private
#undef protected

#include "application_layer.hpp"
#include "transport_layer.hpp"
#include "ip_layer.hpp"
#include "eth_layer.hpp"

namespace BroadInter
{

class Ipv4FragReassemblerTest : public ::testing::Test
{
protected:
	typedef ApplicationLayer<ETHERNET_II, IPV4, TCP, DUMMY> DummyConstructor;
	typedef TransportLayer<ETHERNET_II, IPV4, TCP> TcpConstructor;
	typedef IpLayer<ETHERNET_II, IPV4> IpConstructor;
	typedef EthLayer<ETHERNET_II> EthConstructor;

	Ipv4FragReassemblerTest();
	virtual ~Ipv4FragReassemblerTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();
	
	// Objects declared here can be used by all tests in the test case for Foo.
};

}

#endif

