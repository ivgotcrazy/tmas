/*#############################################################################
 * 文件名   : http_parser_test.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月14日
 * 文件描述 : HttpParserTest声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_PARSER_TEST
#define BROADINTER_HTTP_PARSER_TEST

#include <gtest/gtest.h>

#include "http_parser.hpp"

namespace BroadInter
{

class HttpParserTest : public ::testing::Test
{
protected:
	HttpParserTest();
	virtual ~HttpParserTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp();
	virtual void TearDown();
	
	// Objects declared here can be used by all tests in the test case.

	std::string http_request_;
	HttpRequest request_info_;

	std::string http_response_;
	HttpResponse response_info_;
};

}

#endif


