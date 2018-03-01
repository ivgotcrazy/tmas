/*#############################################################################
 * 文件名   : http_observer_manager.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : HTTP观察器相关类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_OBSERVER_MANAGER
#define BROADINTER_HTTP_OBSERVER_MANAGER

#include <vector>
#include <boost/noncopyable.hpp>

#include "http_filter.hpp"
#include "http_recorder.hpp"
#include "http_typedef.hpp"
#include "http_config_parser.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: HTTP观察者，根据配置记录数据。
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpObserver
{
public:
	HttpObserver(const HttpFilterSP& filter, const HttpRecorderSP& recorder);

	void Process(const HttpRecordInfo& record_info);

private:
	HttpFilterSP filter_;
	HttpRecorderSP recorder_;
};

/******************************************************************************
 * 描述: HTTP观察者管理容器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class HttpObserverManager : public boost::noncopyable
{
public:
	HttpObserverManager(const LoggerSP& logger);

	bool Init();
	void Process(const HttpRecordInfo& record_info);

private:
	HttpFilterSP ConstructHttpFilter(const HttpConfigDomain& config_domain);
	HttpRecorderSP ConstructHttpRecorder(const HttpConfigDomain& config_domain);

private:
	std::vector<HttpObserver> observers_;
	LoggerSP logger_;
};

}

#endif