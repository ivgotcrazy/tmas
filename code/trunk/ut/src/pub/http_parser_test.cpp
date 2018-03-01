/*#############################################################################
 * �ļ���   : http_parser_test.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��14��
 * �ļ����� : ����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "http_parser_test.hpp"

namespace BroadInter
{

#define HTTP_GET	std::string("GET")
#define SPACE		std::string(" ")
#define CR			std::string("\r")
#define LF			std::string("\n")
#define CRLF		std::string("\r\n")
#define HTTP_V10	std::string("HTTP/1.0")
#define HTTP_V11	std::string("HTTP/1.1")

#define STATUS_CODE_200	std::string("200")

#define NORMAL_URI			std::string("/abc/ddd/efs.html")
#define NORMAL_REQUEST_LINE	(HTTP_GET + SPACE + NORMAL_URI + SPACE + HTTP_V11)
#define NORMAL_STATUS_LINE  (HTTP_V11 + SPACE + STATUS_CODE_200 + SPACE + std::string("OK"))
#define NORMAL_HTTP_BODY	std::string("Hello World!")
#define NORMAL_HTTP_HEADER	std::string("Host: pan.baidu.com\r\n" \
                                         "Accept: */*\r\n" \
                                         "Cookie: BDUSS=AAAAAAAAAAAA\r\n" \
					                     "User-Agent: netdisk;4.5.0.7;PC;PC-Windows;6.1.7600;WindowsBaiduYunGuanJia")

#define PARSE_HTTP_REQEUST(http_request_, request_info_) \
	HPN::ParseHttpRequest(http_request_.data(), http_request_.length(), request_info_)

#define PARSE_HTTP_RESPONSE(http_response_, response_info_) \
	HPN::ParseHttpResponse(http_response_.data(), http_response_.length(), response_info_)

#define ASSERT_REQUEST_LINE(request_info_, exp_method, exp_uri, exp_version) \
	ASSERT_TRUE(request_info_.request_line.method == exp_method); \
	ASSERT_TRUE(request_info_.request_line.uri == exp_uri); \
	ASSERT_TRUE(request_info_.request_line.version == exp_version);

#define ASSERT_STATUS_LINE(response_info_, exp_code, exp_version) \
	ASSERT_TRUE(response_info_.status_line.status_code == exp_code); \
	ASSERT_TRUE(response_info_.status_line.version == exp_version);

//==============================================================================
// Test class implementation
//==============================================================================

HttpParserTest::HttpParserTest()
{
}

HttpParserTest::~HttpParserTest()
{
}

void HttpParserTest::SetUp()
{

}

void HttpParserTest::TearDown()
{
	http_request_.clear();
	request_info_.header.clear();

	http_response_.clear();
	response_info_.header.clear();
}

//==============================================================================
// Test case implementation
//==============================================================================

/*------------------------------------------------------------------------------
 * ����: �����Ϸ���HTTP���󡪡���"\r\n"��Ϊ�ָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Normal_1)
{
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            NORMAL_HTTP_HEADER + CRLF + 
				    CRLF +
				    NORMAL_HTTP_BODY;
	ASSERT_TRUE(PARSE_HTTP_REQEUST(http_request_, request_info_));
	ASSERT_REQUEST_LINE(request_info_, HTTP_GET, NORMAL_URI, HTTP_V11);
	ASSERT_TRUE(request_info_.header.size() == 4);
	ASSERT_TRUE(request_info_.header["host"] == "pan.baidu.com");
	ASSERT_TRUE(request_info_.header["accept"] == "*/*");
	ASSERT_TRUE(request_info_.header["cookie"] == "BDUSS=AAAAAAAAAAAA");
	ASSERT_TRUE(request_info_.header["user-agent"] == "netdisk;4.5.0.7;PC;PC-Windows;6.1.7600;WindowsBaiduYunGuanJia");
	ASSERT_TRUE(request_info_.body.len = NORMAL_HTTP_BODY.length());
}

/*------------------------------------------------------------------------------
 * ����: �����Ϸ�HTTP���󡪡���"\n"��Ϊ�ָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Normal_2)
{
	http_request_ = NORMAL_REQUEST_LINE + LF + NORMAL_HTTP_HEADER + LF + LF;
	ASSERT_TRUE(PARSE_HTTP_REQEUST(http_request_, request_info_));
	ASSERT_REQUEST_LINE(request_info_, HTTP_GET, NORMAL_URI, HTTP_V11);
	ASSERT_TRUE(request_info_.header.size() == 4);
	ASSERT_TRUE(request_info_.header["host"] == "pan.baidu.com");
	ASSERT_TRUE(request_info_.header["accept"] == "*/*");
	ASSERT_TRUE(request_info_.header["cookie"] == "BDUSS=AAAAAAAAAAAA");
	ASSERT_TRUE(request_info_.header["user-agent"] == "netdisk;4.5.0.7;PC;PC-Windows;6.1.7600;WindowsBaiduYunGuanJia");
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ҳ���method
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_1)
{
	http_request_ = "aaaaaaaaaaaa";
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡�method��ֻ��space
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_3)
{
	http_request_ = HTTP_GET + SPACE + SPACE + SPACE;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ҳ���URI��β���ָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_4)
{
	http_request_ = HTTP_GET + SPACE + NORMAL_URI;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡�URI�����spaceû����������
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_5)
{
	http_request_ = HTTP_GET + SPACE + NORMAL_URI + SPACE + SPACE;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡�versionû��β���ָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_6)
{
	// TEST7: Can not find version end delim
	http_request_ = HTTP_GET + SPACE + NORMAL_URI + SPACE + HTTP_V11;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡�version��β���ָ���Ϊspace
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_7)
{
	http_request_ = HTTP_GET + SPACE + NORMAL_URI + SPACE + HTTP_V11 + SPACE;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡�����ֻ����request line
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_8)
{
	http_request_ = HTTP_GET + SPACE + NORMAL_URI + SPACE + HTTP_V11 + CRLF;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ƿ���version
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_RequetLine_9)
{
	http_request_ = HTTP_GET + SPACE + NORMAL_URI + SPACE + std::string("HTTP/1.2") + CRLF;
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ҳ���header�ָ�����ֻ��һ��header
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Header_1)
{
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            std::string("Host: pan.baidu.com");
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}
	
/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ҳ���header�ָ���������header
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Header_2)
{
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            std::string("Host: pan.baidu.com\r\nAccept: */*");
	ASSERT_TRUE(!(PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��ظ���header
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Header_3)
{
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            std::string("Host: pan.baidu.com\r\nHost: www.baidu.com") + CRLF + 
				    CRLF;
	ASSERT_TRUE(PARSE_HTTP_REQEUST(http_request_, request_info_));
	ASSERT_EQ(1, request_info_.header.size());
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ҳ���header��body֮��ķָ���"\r\n"
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Header_4)
{
	http_request_ = NORMAL_REQUEST_LINE + CRLF + NORMAL_HTTP_HEADER + CRLF;
	ASSERT_TRUE(PARSE_HTTP_REQEUST(http_request_, request_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ�HTTP���󡪡��Ƿ��ķָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpRequest_Header_5)
{
	// "\n\r"
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            std::string("Host: pan.baidu.com\n\rAccept: */*") + CRLF +
				    CRLF;
	ASSERT_TRUE((PARSE_HTTP_REQEUST(http_request_, request_info_)));

	// "\n\r"
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            std::string("Host: pan.baidu.com\r\nAccept: */*") + LF + CR +
				    CRLF;
	ASSERT_TRUE((PARSE_HTTP_REQEUST(http_request_, request_info_)));

	// "\r"
	http_request_ = NORMAL_REQUEST_LINE + CRLF + 
		            std::string("Host: pan.baidu.com\r\nAccept: */*") + CR +
				    CRLF;
	ASSERT_TRUE((PARSE_HTTP_REQEUST(http_request_, request_info_)));
}

/*------------------------------------------------------------------------------
 * ����: �����Ϸ���HTTP��Ӧ
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_Normal_1)
{
	http_response_ = NORMAL_STATUS_LINE + CRLF + 
		             NORMAL_HTTP_HEADER + CRLF + 
				     CRLF +
				     NORMAL_HTTP_BODY;
	ASSERT_TRUE(PARSE_HTTP_RESPONSE(http_response_, response_info_));
	ASSERT_STATUS_LINE(response_info_, 200, HTTP_V11);
	ASSERT_TRUE(response_info_.header.size() == 4);
	ASSERT_TRUE(response_info_.header["host"] == "pan.baidu.com");
	ASSERT_TRUE(response_info_.header["accept"] == "*/*");
	ASSERT_TRUE(response_info_.header["cookie"] == "BDUSS=AAAAAAAAAAAA");
	ASSERT_TRUE(response_info_.header["user-agent"] == "netdisk;4.5.0.7;PC;PC-Windows;6.1.7600;WindowsBaiduYunGuanJia");
	ASSERT_TRUE(response_info_.body.len = NORMAL_HTTP_BODY.length());
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ҳ���version
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_1)
{
	http_response_ = "aaaaaaaaaaaaaa";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ƿ���version
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_2)
{
	http_response_ = std::string("HTTP/1.2") + SPACE + STATUS_CODE_200;
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ҳ���versionβ���ָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_3)
{
	http_response_ = "HTTP/1.1aaaaaaaaaa";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ҳ���״̬���ķָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_4)
{
	http_response_ = "HTTP/1.1 200aaaaaaaaaaaaaa";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ƿ���״̬��
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_5)
{
	http_response_ = "HTTP/1.1 abc OK";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ҳ���reason��ķָ���
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_6)
{
	http_response_ = "HTTP/1.1 200 OKaaaaaaaaaaa  ";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ����������״̬��
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_7)
{
	http_response_ = NORMAL_STATUS_LINE + CRLF;
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ����version��ֻ��space
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_8)
{
	http_response_ = "HTTP/1.1     ";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ����status code��ֻ��space
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_9)
{
	http_response_ = "HTTP/1.1 200       ";
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

/*------------------------------------------------------------------------------
 * ����: �����Ƿ���HTTP��Ӧ�����Ƿ���header
 *----------------------------------------------------------------------------*/
TEST_F(HttpParserTest, ParseHttpResponse_StatusLine_10)
{
	http_response_ = NORMAL_STATUS_LINE + CRLF +
		             std::string("Host: pan.baidu.com\r\nAccept: */*");
	ASSERT_TRUE(!PARSE_HTTP_RESPONSE(http_response_, response_info_));
}

}