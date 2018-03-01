/*#############################################################################
 * �ļ���   : pkt_distributor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��19��
 * �ļ����� : PktDistributor��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "pkt_distributor.hpp"
#include "pkt_dispatcher.hpp"
#include "pkt_capturer.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] index ȫ�ַ���ķַ�������
 *         [in] device �ַ�����Ӧ��ץ������
 *         [in] receivers ���Ľ�����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
PktDistributor::PktDistributor(uint8 index, const string& device, 
	PktReceiverVec& receivers)
	: distributor_index_(index)
	, device_name_(device)
	, pkt_receivers_(receivers)
{
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool PktDistributor::Init()
{
	pkt_dispatcher_.reset(new PktDispatcher(distributor_index_, pkt_receivers_));
	TMAS_ASSERT(pkt_dispatcher_);
	
	if (!pkt_dispatcher_->Init())
	{
		LOG(ERROR) << "Fail to initialize packet dispatcher";
		return false;
	}

	pkt_capturer_.reset(new PktCapturer(device_name_, pkt_dispatcher_));
	TMAS_ASSERT(pkt_dispatcher_);

	if (!pkt_capturer_->Init())
	{
		LOG(ERROR) << "Fail to initialize packet capturer " << device_name_;
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������ķַ�������ʼ������ץȡ���Ĳ����зַ�
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDistributor::Start()
{
	LOG(INFO) << "Start packet distributor " << (uint16)distributor_index_;

	pkt_capturer_->Start();
}

/*-----------------------------------------------------------------------------
 * ��  ��: ֹͣ���ķַ���
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void PktDistributor::Stop()
{
	pkt_capturer_->Stop();
}

}