/*#############################################################################
 * 文件名   : cl_http_recombinder_test.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月04日
 * 文件描述 : 基于"content-length"的HTTP报文重组相关实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <boost/bind.hpp>

#include "cl_http_recombinder_test.hpp"

namespace BroadInter
{



//==============================================================================
// Test class implementation
//==============================================================================

ClHttpRecombinderTest::ClHttpRecombinderTest()
	: recombinder_(boost::bind(&ClHttpRecombinderTest::ReceviePacket, this, _1, _2, _3))
{

}

ClHttpRecombinderTest::~ClHttpRecombinderTest()
{

}

void ClHttpRecombinderTest::SetUp()
{
	call_result_ = false;
	called_times_ = 0;
	pkt_buf_.clear();
}

void ClHttpRecombinderTest::TearDown()
{

}

void ClHttpRecombinderTest::ReceviePacket(bool result, const char* data, uint32 len)
{
	called_times_++;
	call_result_ = result;

	if (result)
	{
		pkt_buf_.append(data, len);
	}
}

//==============================================================================
// Test case implementation
//==============================================================================

/*------------------------------------------------------------------------------
 * 描述: 空消body
 *----------------------------------------------------------------------------*/
TEST_F(ClHttpRecombinderTest, empty_body)
{
	recombinder_.Init(0);

	recombinder_.AppendData(nullptr, 0);

	ASSERT_EQ(1, called_times_);
	ASSERT_EQ(0, pkt_buf_.length());
}

/*------------------------------------------------------------------------------
 * 描述: 一个消息承载body
 *----------------------------------------------------------------------------*/
TEST_F(ClHttpRecombinderTest, body_on_one_messge)
{
	recombinder_.Init(100);

	char* tmp_buf = new char[100];
	std::memset(tmp_buf, 0xFF, 100);

	recombinder_.AppendData(tmp_buf, 100);

	ASSERT_EQ(1, called_times_);
	ASSERT_EQ(100, pkt_buf_.length());
	ASSERT_EQ(0xFF, (uint8)pkt_buf_.c_str()[0]);

	delete[] tmp_buf;
}

/*------------------------------------------------------------------------------
 * 描述: 两个消息承载body
 *----------------------------------------------------------------------------*/
TEST_F(ClHttpRecombinderTest, body_on_two_messages)
{
	recombinder_.Init(200);

	char* tmp_buf = new char[100];
	std::memset(tmp_buf, 0xFF, 100);

	recombinder_.AppendData(tmp_buf, 100);

	ASSERT_EQ(0, called_times_);
	ASSERT_EQ(0, pkt_buf_.length());

	recombinder_.AppendData(tmp_buf, 100);

	ASSERT_EQ(1, called_times_);
	ASSERT_EQ(200, pkt_buf_.length());
	ASSERT_EQ(0xFF, (uint8)pkt_buf_.c_str()[0]);

	delete[] tmp_buf;
}

}