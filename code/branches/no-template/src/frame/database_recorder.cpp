/*#############################################################################
 * 文件名   : database_recorder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年01月12日
 * 文件描述 : DatabaseRecorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "database_recorder.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 获取单例
 * 参  数: 
 * 返回值: 单例
 * 修  改: 
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
DatabaseRecorder& DatabaseRecorder::GetInstance()
{
	static DatabaseRecorder recorder;
	return recorder;
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改: 
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 添加连接记录
 * 参  数: [in] record 记录
 * 返回值: 
 * 修  改: 
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void DatabaseRecorder::AddConnRecord(const ConnRecord& record)
{

}

/*-----------------------------------------------------------------------------
 * 描  述: 添加HTTP会话记录
 * 参  数: [in] record 记录
 * 返回值: 
 * 修  改: 
 *   时间 2014年01月13日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void DatabaseRecorder::AddHttpSessionRecord(const HttpSessionRecord& record)
{

}

}