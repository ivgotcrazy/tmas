/*#############################################################################
 * �ļ���   : l4_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : L4Monitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_L4_MONITOR
#define BROADINTER_L4_MONITOR

#include <boost/noncopyable.hpp>

#include "pkt_processor.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ��������ݼ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class L4Monitor : public PktProcessor
{
public:
	bool Init();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;
	
private:
	bool ResolveL4Info(PktMsgSP& pkt_msg);
	uint8 GetPktDirection(const PktMsgSP& pkt_msg);

private:
	PktProcessorSP pkt_processor_;
};

}

#endif