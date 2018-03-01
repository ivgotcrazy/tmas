/*#############################################################################
 * �ļ���   : logger.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��06��
 * �ļ����� : ���ݼ�¼����ӿ�����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_LOGGER
#define BROADINTER_LOGGER

#include "record.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���ݼ�¼������ӿ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��05��06��
 ******************************************************************************/
class Logger : public boost::noncopyable
{
public:
	virtual bool Init() = 0;

	virtual void LogTcpHsTimeoutRecord(const TcpHsTimeoutRecord& record) = 0;

	virtual void LogTcpHsDelayRecord(const TcpHsDelayRecord& record) = 0;

	virtual void LogTcpConnTimeoutRecord(const TcpConnTimeoutRecord& record) = 0;

	virtual void LogTcpConnAbortRecord(const TcpConnAbortRecord& record) = 0;

	virtual void LogHttpRespDelayRecord(const HttpRespDelayRecord& record) = 0;

	virtual void LogHttpDlSpeedRecord(const HttpDlSpeedRecord& record) = 0;

	virtual void LogHttpAccessRecord(const HttpAccessRecord& record) = 0;
};


}

#endif