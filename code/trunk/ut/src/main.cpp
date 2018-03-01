/*#############################################################################
 * 文件名   : main.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月18日
 * 文件描述 : 入口
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <gtest/gtest.h>
#include <glog/logging.h>

int main(int argc, char** argv)
{
    FLAGS_minloglevel = 3; // 关闭glog非必要打印

    ::testing::InitGoogleTest(&argc, argv);    

    return RUN_ALL_TESTS();
}
