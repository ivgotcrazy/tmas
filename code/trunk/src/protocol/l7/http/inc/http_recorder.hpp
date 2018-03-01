/*#############################################################################
 * �ļ���   : http_recorder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��30��
 * �ļ����� : HttpRecorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RECORDER
#define BROADINTER_HTTP_RECORDER

#include <boost/noncopyable.hpp>

#include "tmas_typedef.hpp"
#include "http_typedef.hpp"

namespace BroadInter
{

using std::string;

/******************************************************************************
 * ����: HTTP��¼������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class HttpRecorder : public boost::noncopyable
{
public:
	void Record(const HttpRecordInfo& record_info);

	void SetNextRecorder(const HttpRecorderSP& next) { next_recorder_ = next; }

private:
	virtual void DoRecord(const HttpRecordInfo& record_info) = 0;

private:
	HttpRecorderSP next_recorder_;
};

/******************************************************************************
 * ����: HTTP��Ӧʱ�Ӽ�¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class HttpRespDelayRecorder : public HttpRecorder
{
public:
	HttpRespDelayRecorder(const LoggerSP& logger, uint32 delay);

private:
	virtual void DoRecord(const HttpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 resp_delay_;
};

/******************************************************************************
 * ����: HTTP�����ٶȼ�¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
 *****************************************************************************/
class HttpDlSpeedRecorder : public HttpRecorder
{
public:
	HttpDlSpeedRecorder(const LoggerSP& logger, uint32 speed);

private:
	virtual void DoRecord(const HttpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
	uint32 dl_speed_;
};

/******************************************************************************
 * ����: HTTP���ʼ�¼��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��12��
 *****************************************************************************/
class HttpAccessRecorder : public HttpRecorder
{
public:
	HttpAccessRecorder(const LoggerSP& logger);
	
private:
	virtual void DoRecord(const HttpRecordInfo& record_info) override;

private:
	LoggerSP logger_;
};

}

#endif
