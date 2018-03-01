/*#############################################################################
 * �ļ���   : eth_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��21��
 * �ļ����� : EthMonitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_ETH_MONITOR
#define BROADINTER_ETH_MONITOR

#include "pkt_processor.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ETH�����ݼ�������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��21��
 ******************************************************************************/
class EthMonitor : public PktProcessor
{
public:
	bool Init();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	void PrintEthInfo(const PktMsgSP& pkt_msg);
};

}

#endif
