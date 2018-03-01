/*#############################################################################
 * 文件名   : ip_monitor_test.hpp
 * 创建人   : tom_liu
 * 创建时间 : 2014年02月20日
 * 文件描述 : 声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_IP_MONITOR_TEST
#define BROADINTER_IP_MONITOR_TEST

#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "ip_monitor.hpp"
#include "application_layer.hpp"
#include "transport_layer.hpp"
#include "ip_layer.hpp"
#include "eth_layer.hpp"

#undef private
#undef protected

namespace BroadInter
{

class IpMonitorTest : public ::testing::Test
{
protected:
	typedef ApplicationLayer<ETHERNET_II, IPV4, TCP, DUMMY> DummyConstructor;
	typedef TransportLayer<ETHERNET_II, IPV4, TCP> TcpConstructor;
	typedef IpLayer<ETHERNET_II, IPV4> IpConstructor;
	typedef EthLayer<ETHERNET_II> EthConstructor;

	IpMonitorTest();
	virtual ~IpMonitorTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();
	
	// Objects declared here can be used by all tests in the test case for Foo.

	IpMonitor ip_monitor_;

	//PktConstructor pkt_constructor_;
};

}

#endif

