/*#############################################################################
 * �ļ���   : cl_http_recombinder_test.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��04��
 * �ļ����� : ����"content-length"��HTTP���������������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CL_HTTP_RECOMBINDER_TEST
#define BROADINTER_CL_HTTP_RECOMBINDER_TEST

#include <gtest/gtest.h>

#define private public
#define protected public
#include "http_recombinder_impl.hpp"
#undef private
#undef protected

namespace BroadInter
{

class ClHttpRecombinderTest : public ::testing::Test
{
protected:
	ClHttpRecombinderTest();
	virtual ~ClHttpRecombinderTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();

	// Objects declared here can be used by all tests in the test case.

	void ReceviePacket(bool result, const char* data, uint32 len);

	uint32 called_times_;
	std::string pkt_buf_;
	bool call_result_;

	ContentLengthRecombinder recombinder_;
};

}

#endif