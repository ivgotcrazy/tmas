/*#############################################################################
 * �ļ���   : tcp_recorder.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : TcpReorder��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "tcp_recorder.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ��¼������Ϣ
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void TcpRecorder::Record(const TcpRecordInfo& record_info)
{
	DoRecord(record_info);

	if (next_recorder_)
	{
		next_recorder_->Record(record_info);
	}
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 *         [in] delay ʱ����ֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HsDelayRecorder::HsDelayRecorder(const LoggerSP& logger, uint32 delay)
	: logger_(logger), delay_(delay)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼������Ϣ
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HsDelayRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// ���������رղ���Ҫ��¼����ʱ��
	if (record_info.conn_event != TCE_CONN_CLOSE) return;

	// ֻ��¼��Ԥ��ֵ�������ʱ��
	if (record_info.hs_delay < delay_) return;

	TcpHsDelayRecord delay_record;

	delay_record.conn_id = record_info.conn_id;
	delay_record.hs_delay = record_info.hs_delay;
	delay_record.record_time = 0;

	logger_->LogTcpHsDelayRecord(delay_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 *         [in] timeout ��ʱֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HsTimeoutRecorder::HsTimeoutRecorder(const LoggerSP& logger, uint32 timeout)
	: logger_(logger), timeout_(timeout)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼������Ϣ
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HsTimeoutRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// ֻ��ע���ֳ�ʱ�¼�
	if (record_info.conn_event != TCE_HS_TIMEOUT) return;

	TcpHsTimeoutRecord timeout_record;

	timeout_record.conn_id = record_info.conn_id;
	timeout_record.timeout_value = timeout_;
	timeout_record.record_time = 0;

	logger_->LogTcpHsTimeoutRecord(timeout_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
ConnAbortRecorder::ConnAbortRecorder(const LoggerSP& logger) : logger_(logger)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼������Ϣ
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void ConnAbortRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// ֻ��עTCP�����쳣�ж��¼�
	if (record_info.conn_event != TCE_CONN_ABORT) return;

	TcpConnAbortRecord abort_record;

	abort_record.conn_id = record_info.conn_id;
	abort_record.record_time = 0;

	logger_->LogTcpConnAbortRecord(abort_record);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 *         [in] timeout ��ʱֵ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
ConnTimeoutRecorder::ConnTimeoutRecorder(const LoggerSP& logger, uint32 timeout)
	: logger_(logger), timeout_(timeout)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��¼������Ϣ
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void ConnTimeoutRecorder::DoRecord(const TcpRecordInfo& record_info)
{
	// ֻ��עTCP���ӳ�ʱ�¼�
	if (record_info.conn_event != TCE_CONN_TIMEOUT) return;

	TcpConnTimeoutRecord timeout_record;

	timeout_record.conn_id = record_info.conn_id;
	timeout_record.timeout_value = timeout_;
	timeout_record.record_time = 0;

	logger_->LogTcpConnTimeoutRecord(timeout_record);
}

}