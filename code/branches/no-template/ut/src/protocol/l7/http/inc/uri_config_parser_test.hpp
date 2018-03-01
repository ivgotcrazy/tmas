/*##############################################################################
 * 文件名   : uri_config_parser_test.hpp 
 * 创建人   : rosan 
 * 创建时间 : 2014.02.21
 * 文件描述 : UriConfigParser类的测试类 
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#ifndef BROADINTER_URI_CONFIG_PARSER_TEST
#define BROADINTER_URI_CONFIG_PARSER_TEST

#include <string>
#include <gtest/gtest.h>

#include "uri_config_parser.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: UriConfigParser类的测试类
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
class UriConfigParserTest : public ::testing::Test
{
protected:
    virtual void SetUp() override;

protected:
    bool ok_;  // 解析配置文件是否成功
    size_t line_;  // 解析配置文件失败后的错误行号
    std::string description_;  // 解析配置文件失败后的错误描述
    UriConfigParser parser_;  // 用于解析配置文件的对象
};

}  // namespace BroadInter

#endif  // BROADINTER_URI_CONFIG_PARSER_TEST
