/*#############################################################################
 * �ļ���   : database_recorder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��12��
 * �ļ����� : DatabaseRecorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_DATABASE_RECORDER
#define BROADINTER_DATABASE_RECORDER

#include <boost/noncopyable.hpp>

#include <mysql/mysql.h>

#include "record.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ��װMysql���ݿ��¼��д
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��12��
 ******************************************************************************/
class DatabaseRecorder : public boost::noncopyable
{
public:
	static DatabaseRecorder& GetInstance();

	bool Init();

	void AddConnRecord(const ConnRecord& record);
	void AddHttpSessionRecord(const HttpSessionRecord& record);

private:
	DatabaseRecorder() {}
	DatabaseRecorder(const DatabaseRecorder& recorder);

private:
	MYSQL mysql_; // mysql���ݿ�
};

}

#endif