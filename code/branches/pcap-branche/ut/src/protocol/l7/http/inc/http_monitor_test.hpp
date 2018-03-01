/*##############################################################################
 * 文件名   : http_monitor_test.hpp
 * 创建人   : rosan 
 * 创建时间 : 2014.02.24
 * 文件描述 : HttpMonitor类的测试类声明文件 
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#ifndef BROADINTER_HTTP_MONITOR_TEST
#define BROADINTER_HTTP_MONITOR_TEST

#include <boost/smart_ptr.hpp>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "http_monitor.hpp"
#undef private
#undef protected

namespace BroadInter
{

/*******************************************************************************
 * 描  述: HttpMonitor类的测试类
 * 作  者: rosan
 * 时  间: 2013.02.24
 ******************************************************************************/
class HttpMonitorTest : public ::testing::Test
{
protected:
    virtual void SetUp() override;

protected:
    boost::shared_ptr<void> ptr_;
    PktMsg* msg_;
    boost::shared_ptr<HttpMonitor> http_monitor_;  // 待测试的对象
};

}  // namespace BroadInter

#endif  // BROADINTER_HTTP_MONITOR_TEST
