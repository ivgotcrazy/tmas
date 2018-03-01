/*#############################################################################
 * 文件名   : database_recorder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月12日
 * 文件描述 : DatabaseRecorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_DATABASE_RECORDER
#define BROADINTER_DATABASE_RECORDER

#include <boost/noncopyable.hpp>

#include <mysql/mysql.h>

#include "record.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 封装Mysql数据库记录读写
 * 作  者: teck_zhou
 * 时  间: 2014年01月12日
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
	MYSQL mysql_; // mysql数据库
};

}

#endif