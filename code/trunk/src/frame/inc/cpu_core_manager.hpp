/*#############################################################################
 * �ļ���   : cpu_core_manager.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��21��
 * �ļ����� : CpuCoreManager������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CPU_CORE_MANAGER
#define BROADINTER_CPU_CORE_MANAGER

#include <boost/thread.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: CPU���Ĺ�����������ͳһ����CPU���İ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��04��21��
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