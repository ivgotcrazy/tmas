/*#############################################################################
 * 文件名   : cpu_core_manager.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年04月21日
 * 文件描述 : CpuCoreManager类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CPU_CORE_MANAGER
#define BROADINTER_CPU_CORE_MANAGER

#include <boost/thread.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * 描  述: CPU核心管理器，用来统一调度CPU核心绑定
 * 作  者: teck_zhou
 * 时  间: 2014年04月21日
 ******************************************************************************/
class CpuCoreManager
{
public:
	static CpuCoreManager& GetInstance();

	bool Init();

	bool GetUnboundCpuCore(uint8& cpu_core);

private:
	CpuCoreManager() {}

private:
	std::vector<bool> cpu_cores_;

	boost::mutex m_;
};

}

#endif