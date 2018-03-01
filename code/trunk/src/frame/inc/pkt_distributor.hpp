/*#############################################################################
 * �ļ���   : pkt_distributor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��04��19��
 * �ļ����� : PktDistributor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_DISTRIBUTOR
#define BROADINTER_PKT_DISTRIBUTOR

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * ��  ��: �����ĵĲ�����ٷַ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��04��19��
 ******************************************************************************/
class PktDistributor : public boost::noncopyable
{
public:
	PktDistributor(uint8 index, 
		           const string& device, 
				   PktReceiverVec& receivers);

	bool Init();

	void Start();
	void Stop();

private:
	// ȫ�ַ��������
	uint8 distributor_index_;

	// ץ������
	std::string device_name_;

	// ���Ĳ�����
	PktCapturerSP pkt_capturer_;

	// ���ķַ���
	PktDispatcherSP pkt_dispatcher_;

	// ����PktDispatcherʹ��
	PktReceiverVec& pkt_receivers_;
};

}

#endif