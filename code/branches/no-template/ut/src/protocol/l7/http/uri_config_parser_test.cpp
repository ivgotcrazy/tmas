/*##############################################################################
 * 文件名   : uri_config_parser_test.cpp 
 * 创建人   : rosan 
 * 创建时间 : 2014.02.21
 * 文件描述 : UriConfigParser类的单元测试 
 * 版权声明 : Copyright(c)2013 BroadInter.All rights reserved.
 * ###########################################################################*/
#include "uri_config_parser_test.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 每个测试用例开始执行前的初始化工作
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
void UriConfigParserTest::SetUp()
{
    ok_ = false;
    line_ = 0;
    description_.clear();
}

/*******************************************************************************
 * 描  述: 测试解析正常的配置文件
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
TEST_F(UriConfigParserTest, normal)
{
    parser_.Parse("../config/uri_normal.conf", &ok_, &line_, &description_);
    EXPECT_TRUE(ok_);
    EXPECT_TRUE(description_.empty());
}

/*******************************************************************************
 * 描  述: 测试待解析的配置文件不存在的情况
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
TEST_F(UriConfigParserTest, file_not_exist)
{
    auto uri = parser_.Parse("../config/uri_file_not_exist.conf", &ok_, &line_, &description_);
    EXPECT_TRUE(uri.empty());
}

/*******************************************************************************
 * 描  述: 测试待解析的配置文件中左括号缺失的情况
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
TEST_F(UriConfigParserTest, uri_loss_left_braces)
{
    parser_.Parse("../config/uri_loss_left_braces.conf", &ok_, &line_, &description_);
    EXPECT_TRUE(!ok_);
    EXPECT_TRUE(!description_.empty());
}

/*******************************************************************************
 * 描  述: 测试待解析的配置文件中右括号缺失的情况
 * 作  者: rosan
 * 时  间: 2013.02.21
 ******************************************************************************/
TEST_F(UriConfigParserTest, uri_loss_right_braces)
{
    parser_.Parse("../config/uri_loss_right_braces.conf", &ok_, &line_, &description_);
    EXPECT_TRUE(!ok_);
    EXPECT_TRUE(!description_.empty());
}

}  // namespace BroadInter
