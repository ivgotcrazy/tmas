/*#############################################################################
 * �ļ���   : main.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��18��
 * �ļ����� : ���
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <gtest/gtest.h>
#include <glog/logging.h>

int main(int argc, char** argv)
{
    FLAGS_minloglevel = 3; // �ر�glog�Ǳ�Ҫ��ӡ

    ::testing::InitGoogleTest(&argc, argv);    

    return RUN_ALL_TESTS();
}
