/*#############################################################################
 * 文件名   : chunked_http_recombinder_test.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月04日
 * 文件描述 : HTTP报文重组相关声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CHUNKED_HTTP_RECOMBINDER_TEST
#define BROADINTER_CHUNKED_HTTP_RECOMBINDER_TEST

#include <gtest/gtest.h>

#define private public
#define protected public
#include "http_recombinder_impl.hpp"
#undef private
#undef protected

namespace BroadInter
{

class ChunkedHttpRecombinderTest : public ::testing::Test
{
protected:
	ChunkedHttpRecombinderTest();
	virtual ~ChunkedHttpRecombinderTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();

	// Objects declared here can be used by all tests in the test case.

	void ReceviePacket(bool result, const char* data, uint32 len);

	uint32 called_times_;
	std::string pkt_buf_;
	bool call_result_;

	ChunkedRecombinder recombinder_;
};

}

#endif