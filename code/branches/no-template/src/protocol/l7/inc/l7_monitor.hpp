/*#############################################################################
 * �ļ���   : l7_monitor.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : L7Monitor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_L7_MONITOR
#define BROADINTER_L7_MONITOR

#include <boost/noncopyable.hpp>

#include "pkt_processor.hpp"
#include "tmas_typedef.hpp"
#include "message.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: Ӧ�ò����ݼ���
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class L7Monitor : public PktProcessor
{
public:
	bool Init();

private:
	virtual ProcInfo DoProcess(MsgType msg_type, VoidSP msg_data) override;

private:
	PktProcessorSP pkt_processor_;
};

}

#endif