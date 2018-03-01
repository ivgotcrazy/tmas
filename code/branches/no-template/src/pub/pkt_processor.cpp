/*#############################################################################
 * �ļ���   : pkt_processor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : PktProcessor������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tmas_assert.hpp"
#include "pkt_processor.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ��ͷ�ļ�����
 * ��  ��:
 *   ʱ�� 2014��01��11��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
ProcInfo PktProcessor::Process(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_data);

	ProcInfo ret = DoProcess(msg_type, msg_data);
	if (!(ret & PI_CHAIN_CONTINUE))
	{
		return ret; // ���������ȷ���ټ������ݱ����򷵻�
	}

	// �ߵ����˵����Ҫ���¼�������
	if (!successor_)
	{
		DLOG(WARNING) << "Fail to find processor to process packet";
		return PI_RET_STOP;
	}
	else
	{
		return successor_->Process(msg_type, msg_data);
	}
}

}