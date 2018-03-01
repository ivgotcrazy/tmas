/*#############################################################################
 * 文件名   : cpu_core_manager.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月21日
 * 文件描述 : CpuCoreManager类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <stdio.h>
#include <unistd.h>

#include <glog/logging.h>

#include "cpu_core_manager.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 获取单例
 * 参  数: 
 * 返回值: 单例
 * 修  改:
 *   时间 2014年04月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
CpuCoreManager& CpuCoreManager::GetInstance()
{
	static CpuCoreManager manager;
	return manager;
}

/*-----------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool CpuCoreManager::Init()
{
	/* 
	* - _SC_NPROCESSORS_CONF
	*       The number of processors configured.
	* 
	* - _SC_NPROCESSORS_ONLN
	*       The number of processors currently online (available).
	*/ 

	int cpu_num;

	//cpu_num = sysconf(_SC_NPROCESSORS_CONF);

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	if (cpu_num <= 0)
	{
		LOG(ERROR) << "Invalid cpu core number : " << cpu_num;
		return false;
	}

	LOG(INFO) << "Cpu core number : " << cpu_num;

	for (int i = 0; i < cpu_num; i++)
	{
		cpu_cores_.push_back(true);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取未绑定的CPU核心
 * 参  数: [out] cpu_core CPU核心索引
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月21日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool CpuCoreManager::GetUnboundCpuCore(uint8& cpu_core)
{
	// 为了保证极端情况系统所有的CPU资源都被耗尽，CPU核心0不分配

	boost::mutex::scoped_lock lock(m_);

	for (uint8 i = 1; i < cpu_cores_.size(); i++)
	{
		if (cpu_cores_[i])
		{
			cpu_core = i;
			cpu_cores_[i] = false;

			return true;
		}
	}

	return false;
}

}