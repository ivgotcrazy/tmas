/*#############################################################################
 * �ļ���   : database_recorder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : DatabaseRecorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "database_recorder.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ����
 * ��  ��: 
 * ����ֵ: ����
 * ��  ��: 
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
DatabaseRecorder& DatabaseRecorder::GetInstance()
{
	static DatabaseRecorder recorder;
	return recorder;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool DatabaseRecorder::Init()
{
	if (!mysql_init(&mysql_))
	{
		LOG(ERROR) << "Fail to init mysql";
		return false;
	}

	if (!mysql_real_connect(&mysql_, "192.168.0.191", "teck_zhou", 
		"", "test", 3306, NULL, 0))
	{
		LOG(ERROR) << "Fail to connect mysql | " << mysql_error(&mysql_);
		return false;
	}

	DLOG(INFO) << "Initialize mysql successfully";

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������Ӽ�¼
 * ��  ��: [in] record ��¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseRecorder::AddConnRecord(const ConnRecord& record)
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ���HTTP�Ự��¼
 * ��  ��: [in] record ��¼
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��01��13��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void DatabaseRecorder::AddHttpSessionRecord(const HttpSessionRecord& record)
{

}

}