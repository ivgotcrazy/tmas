/*#############################################################################
 * �ļ���   : l7_monitor.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��01��02��
 * �ļ����� : L7Monitor��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "l7_monitor.hpp"
#include "tmas_assert.hpp"
#include "http_monitor.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��: 
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool L7Monitor::Init()
{
    HttpMonitor* http_monitor = new HttpMonitor;
    pkt_processor_.reset(http_monitor);

    if (!http_monitor->Init())
    {
        DLOG(INFO) << "Fail to init HTTP protocol monitor.";
        return false;
    }

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ:
 * ��  ��: 
 *   ʱ�� 2014��01��08��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
ProcInfo L7Monitor::DoProcess(MsgType msg_type, VoidSP msg_data)
{
	TMAS_ASSERT(msg_data);
	TMAS_ASSERT(pkt_processor_);

	// �����Ӵ�����
	pkt_processor_->Process(msg_type, msg_data);

	// ����L7Monitor�Ѿ�������������ĩ�ˣ�����Ҫ�ٽ��������´���
	return PI_RET_STOP;
}

}
