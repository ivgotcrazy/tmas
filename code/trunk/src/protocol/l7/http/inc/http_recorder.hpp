/*#############################################################################
 * 文件名   : http_recorder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月30日
 * 文件描述 : HttpRecorder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描述: HTTP记录器基类
 * 作者：teck_zhou
 * 时间：2014年05月08日
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
 * 描述: HTTP响应时延记录器
 * 作者：teck_zhou
 * 时间：2014年05月08日
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
 * 描述: HTTP下载速度记录器
 * 作者：teck_zhou
 * 时间：2014年05月08日
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
 * 描述: HTTP访问记录器
 * 作者：teck_zhou
 * 时间：2014年05月12日
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
