/*#############################################################################
 * �ļ���   : tcp_recorder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : TcpReorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_RECORDER
#define BROADINTER_TCP_RECORDER

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "connection.hpp"
#include "tcp_typedef.hpp"
#include "logger.hpp"

namespace BroadInter
{

/******************************************************************************
 * ����: TCP��¼������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class TcpRecorder : public boost::noncopyable
{
public:
	void Record(const TcpRecordInfo& record_info);

	void SetNextRecorder(const TcpRecorderSP& next) { next_recorder_ = next; }

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) = 0;

private:
	TcpRecorderSP next_recorder_;
};

/******************************************************************************
 * ����: TCP����ʱ�Ӽ�¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class HsDelayRecorder : public TcpRecorder
{
public:
	HsDelayRecorder(const LoggerSP& logger, uint32 delay);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 delay_;
};

/******************************************************************************
 * ����: TCP���ֳ�ʱ��¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class HsTimeoutRecorder : public TcpRecorder
{
public:
	HsTimeoutRecorder(const LoggerSP& logger, uint32 timeout);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 timeout_;
};

/******************************************************************************
 * ����: TCP�����쳣�Ͽ���¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class ConnAbortRecorder : public TcpRecorder
{
public:
	ConnAbortRecorder(const LoggerSP& logger);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
};

/******************************************************************************
 * ����: TCP���ӳ�ʱ��¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class ConnTimeoutRecorder : public TcpRecorder
{
public:
	ConnTimeoutRecorder(const LoggerSP& logger, uint32 timeout);

private:
	virtual void DoRecord(const TcpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 timeout_;
};

}

#endif