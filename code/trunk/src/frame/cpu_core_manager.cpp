/*#############################################################################
 * �ļ���   : cpu_core_manager.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��21��
 * �ļ����� : CpuCoreManager������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <stdio.h>
#include <unistd.h>

#include <glog/logging.h>

#include "cpu_core_manager.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ����
 * ��  ��: 
 * ����ֵ: ����
 * ��  ��:
 *   ʱ�� 2014��04��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
CpuCoreManager& CpuCoreManager::GetInstance()
{
	static CpuCoreManager manager;
	return manager;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��21��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ��ȡδ�󶨵�CPU����
 * ��  ��: [out] cpu_core CPU��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��21��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool CpuCoreManager::GetUnboundCpuCore(uint8& cpu_core)
{
	// Ϊ�˱�֤�������ϵͳ���е�CPU��Դ�����ľ���CPU����0������

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