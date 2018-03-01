/*#############################################################################
 * 文件名   : http_observer_manager.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : HTTP观察器相关类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_observer_manager.hpp"
#include "http_config_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] filter 过滤器
 *         [in] recorder 记录器
 * 返回值:
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpObserver::HttpObserver(const HttpFilterSP& filter, const HttpRecorderSP& recorder)
	: filter_(filter), recorder_(recorder)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 如果匹配过滤器，则记录相关信息
 * 参  数: [in] record_info 记录信息
 * 返回值:
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpObserver::Process(const HttpRecordInfo& record_info)
{
	if (filter_->Match(record_info))
	{
		recorder_->Record(record_info);
	}
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] logger 记录器
 * 返回值:
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpObserverManager::HttpObserverManager(const LoggerSP& logger) 
	: logger_(logger)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 初始化
 * 参  数: 
 * 返回值:
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool HttpObserverManager::Init()
{
	HttpConfigDomainVec http_domains; 
	if (!HttpConfigParser::ParseHttpConfig(http_domains))
	{
		LOG(ERROR) << "Fail to parse http config";
		return false;
	}

	for (HttpConfigDomain& http_domain : http_domains)
	{
		HttpFilterSP filter = ConstructHttpFilter(http_domain);
		HttpRecorderSP recorder = ConstructHttpRecorder(http_domain);

		observers_.push_back(HttpObserver(filter, recorder));
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 根据配置构建HTTP过滤器
 * 参  数: [in] config_domain 配置域信息 
 * 返回值: 过滤器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpFilterSP HttpObserverManager::ConstructHttpFilter(const HttpConfigDomain& config_domain)
{
	HttpFilterSP filter;

	// 构建IP地址过滤器
	if (!config_domain.ip_regions.empty())
	{
		HttpFilterSP ip_fileter(new HttpIpFilter(config_domain.ip_regions));
		ip_fileter->SetDecorator(filter);
		filter = ip_fileter;
	}

	// 构建端口过滤器
	if (!config_domain.port_regions.empty())
	{
		HttpFilterSP port_filter(new HttpPortFilter(config_domain.port_regions));
		port_filter->SetDecorator(filter);
		filter = port_filter;
	}

	// 构建Host过滤器
	if (!config_domain.host_regex.empty())
	{
		HttpFilterSP host_filter(new HttpHostFilter(config_domain.host_regex));
		host_filter->SetDecorator(filter);
		filter = host_filter;
	}

	// 构建URI过滤器
	if (!config_domain.uri_regex.empty())
	{
		HttpFilterSP uri_filter(new HttpUriFilter(config_domain.uri_regex));
		uri_filter->SetDecorator(filter);
		filter = uri_filter;
	}

	return filter;
}

/*------------------------------------------------------------------------------
 * 描  述: 根据配置信息构建记录器
 * 参  数: [in] config_domain 配置域信息
 * 返回值: 记录器
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpRecorderSP HttpObserverManager::ConstructHttpRecorder(const HttpConfigDomain& config_domain)
{
	HttpRecorderSP recorder;

	if (config_domain.dl_speed_monitor)
	{
		HttpRecorderSP speed_recorder(
			new HttpDlSpeedRecorder(logger_, config_domain.dl_speed_max));

		speed_recorder->SetNextRecorder(recorder);
		recorder = speed_recorder;
	}

	if (config_domain.resp_delay_monitor)
	{
		HttpRecorderSP delay_recorder(
			new HttpRespDelayRecorder(logger_, config_domain.resp_delay_min));

		delay_recorder->SetNextRecorder(recorder);
		recorder = delay_recorder;
	}

	if (config_domain.access_monitor)
	{
		HttpRecorderSP access_recorder(new HttpAccessRecorder(logger_));

		access_recorder->SetNextRecorder(recorder);
		recorder = access_recorder;
	}

	return recorder;
}

/*------------------------------------------------------------------------------
 * 描  述: 处理记录信息
 * 参  数: [in] record_info 记录信息
 * 返回值:
 * 修  改:
 *   时间 2014年05月09日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpObserverManager::Process(const HttpRecordInfo& record_info)
{
	for (HttpObserver& observer : observers_)
	{
		observer.Process(record_info);
	}
}

}