/*##############################################################################
 * 文件名   : http_monitor_test.hpp
 * 创建人   : rosan 
 * 创建时间 : 2014.02.24
 * 文件描述 : HttpMonitor类的测试类实现文件 
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#include "http_monitor_test.hpp"
#include <fstream>

namespace BroadInter
{

/*
TEST_F(HttpMonitorTest, interest_uri)
{
    HttpMonitor http_monitor;
    http_monitor.Init();
    std::ofstream ofs("log.txt");
    for (decltype(*http_monitor.interest_domains_.begin()) p : http_monitor.interest_domains_)
    {
        ofs << p.first << '\n';
        for (const std::string& q : p.second)
        {
            ofs << '\t' << q << '\n';
        }
        ofs << '\n';
    }
    ofs.close();

    std::string domain = "www.google.com";
    std::uri = "/dd";

    auto iter = http_monitor.interest_domains_.find(domain);
    EXPECT_TRUE(iter != http_monitor.interest_domains.end());

    EXPECT(iter->second.end() != iter->second.find(uri));

    //EXPECT_TRUE(http_monitor.IsInterestUri("www.google.com", "/dd"));
    //EXPECT_TRUE(http_monitor.IsInterestDomain("www.baidu.com"));
}
*/

/*------------------------------------------------------------------------------
 * 描  述: 测试用例初始化函数
 * 参  数:
 * 返回值:
 * 修  改:
 *   时间 2014.02.24
 *   作者 rosan
 *   描述 创建
 -----------------------------------------------------------------------------*/ 
void HttpMonitorTest::SetUp()
{
    msg_ = new PktMsg;  // 创建一个新的消息
    ptr_.reset(msg_);

    http_monitor_.reset(new HttpMonitor);  // 创建一个新的待测试对象
    http_monitor_->Init();  // 初始化新创建的对象
}

/*******************************************************************************
 * 描  述: 不合法的http数据包
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_unexpected)
{
    char data[] = "unexpected data";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_CHAIN_CONTINUE);
}

/*******************************************************************************
 * 描  述: http request解析失败
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_request_resolve_failure)
{
    char data[] = 
        "GET / HTTP/1.1\r\n"
        "invalid http request";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: http request 查找host首部失败
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_request_find_host_failure)
{
    char data[] = 
        "GET / HTTP/1.1\r\n"
        "Accept: */*\r\n\r\n";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: http request的host不是感兴趣的host
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_request_host_uninterested)
{
    char data[] = 
        "GET / HTTP/1.1\r\n"
        "Host: www.not_interested.com\r\n\r\n";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: http request的host是感兴趣的host
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_request_host_interested)
{
    char data[] = 
        "GET / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n\r\n";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: http重复的request
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_request_duplicated)
{
    char data[] = 
        "GET / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n\r\n";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: 非法的http response
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, invalid_http_response)
{
    char data[] = "HTTP/1.1 200 OK";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: 没有对应请求的http response
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_response_without_corresponding_request)
{
    char data[] = "HTTP/1.1 200 OK\r\n"
                  "Content-type: text/plain\r\n"
                  "\r\n"
                  "hello world\r\n";

    msg_->l7_pkt_info.l7_data = data;
    msg_->l4_pkt_info.l4_data_len = sizeof(data) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: 正常的http response
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, normal_http_response)
{
    char request[] = "GET / HTTP/1.1\r\n"
                    "Host: www.baidu.com\r\n"
                    "\r\n";
    char response[] = "HTTP/1.1 200 OK\r\n"
                  "Content-type: text/plain\r\n"
                  "\r\n"
                  "hello world\r\n";

    msg_->l7_pkt_info.l7_data = request;
    msg_->l4_pkt_info.l4_data_len = sizeof(request) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);

    msg_->l7_pkt_info.l7_data = response;
    msg_->l4_pkt_info.l4_data_len = sizeof(response) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

/*******************************************************************************
 * 描  述: http response非200状态
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
TEST_F(HttpMonitorTest, http_response_status_not_200)
{
    char request[] = "GET / HTTP/1.1\r\n"
                    "Host: www.baidu.com\r\n"
                    "\r\n";
    char response[] = "HTTP/1.1 400 OK\r\n"
                  "Content-type: text/plain\r\n"
                  "\r\n"
                  "hello world\r\n";

    msg_->l7_pkt_info.l7_data = request;
    msg_->l4_pkt_info.l4_data_len = sizeof(request) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);

    msg_->l7_pkt_info.l7_data = response;
    msg_->l4_pkt_info.l4_data_len = sizeof(response) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);

    msg_->l7_pkt_info.l7_data = response;
    msg_->l4_pkt_info.l4_data_len = sizeof(response) - 1;
    EXPECT_EQ(http_monitor_->DoProcess(MSG_PKT, ptr_), PI_RET_STOP);
}

}  // namespace BroadInter
