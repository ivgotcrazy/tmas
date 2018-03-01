/*##############################################################################
 * �ļ���   : http_monitor_test.hpp
 * ������   : rosan 
 * ����ʱ�� : 2014.02.24
 * �ļ����� : HttpMonitor��Ĳ����������ļ� 
 * ��Ȩ���� : Copyright(c)2013 BroadInter.All rights reserved.
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
 * ��  ��: HttpMonitor��Ĳ�����
 * ��  ��: rosan
 * ʱ  ��: 2013.02.24
 ******************************************************************************/
class HttpMonitorTest : public ::testing::Test
{
protected:
    virtual void SetUp() override;

protected:
    boost::shared_ptr<void> ptr_;
    PktMsg* msg_;
    boost::shared_ptr<HttpMonitor> http_monitor_;  // �����ԵĶ���
};

}  // namespace BroadInter

#endif  // BROADINTER_HTTP_MONITOR_TEST
