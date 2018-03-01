/*#############################################################################
 * 文件名   : tcp_observer_manager.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : 观察器相关类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_observer_manager.hpp"
#include "tmas_config_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] filter 过滤器
 *         [in] recorder 记录器
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
TcpObserver::TcpObserver(const TcpFilterSP& filter, const TcpRecorderSP& recorder)
	: filter_(filter), recorder_(recorder)
{

}

/*------------------------------------------------------------------------------
 * 描  述: 处理连接信息(废话)
 * 参  数: [in] conn_info 连接信息
 * 返回值: 
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void TcpObserver::Process(const TcpRecordInfo& record_info)
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
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
TcpObserverManager::TcpObserverManager(const LoggerSP& logger) : logger_(logger)
{
}

/*------------------------------------------------------------------------------
 * 描  述: 初始化(废话)
 * 参  数: 
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
bool TcpObserverManager::Init()
{
	TcpConfigDomainVec tcp_domains; 
	if (!TcpConfigParser::ParseTcpConfig(tcp_domains))
	{
		LOG(ERROR) << "Fail to parse tcp config";
		return false;
	}

	for (TcpConfigDomain& tcp_domain : tcp_domains)
	{
		TcpFilterSP filter = ConstructTcpFilter(tcp_domain);
		TcpRecorderSP recorder = ConstructTcpRecorder(tcp_domain);

		observers_.push_back(TcpObserver(filter, recorder));
	}

	return true;
}

/*------------------------------------------------------------------------------
 * 描  述: 构建过滤器
 * 参  数: [in] config_domain 配置域信息
 * 返回值: 过滤器
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
TcpFilterSP TcpObserverManager::ConstructTcpFilter(const TcpConfigDomain& config_domain)
{
	TcpFilterSP filter;

	// 构建IP地址过滤器
	if (!config_domain.ip_regions.empty())
	{
		TcpFilterSP ip_fileter(new TcpIpFilter(config_domain.ip_regions));
		ip_fileter->SetDecorator(filter);
		filter = ip_fileter;
	}

	// 构建端口过滤器
	if (!config_domain.port_regions.empty())
	{
		TcpFilterSP port_filter(new TcpPortFilter(config_domain.port_regions));
		port_filter->SetDecorator(filter);
		filter = port_filter;
	}

	return filter;
}

/*------------------------------------------------------------------------------
 * 描  述: 构建记录器
 * 参  数: [in] config_domain 配置域信息
 * 返回值: 记录器
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
TcpRecorderSP TcpObserverManager::ConstructTcpRecorder(const TcpConfigDomain& config_domain)
{
	TcpRecorderSP recorder;

	if (config_domain.conn_abort_monitor)
	{
		TcpRecorderSP abort_recorder(new ConnAbortRecorder(logger_));
		abort_recorder->SetNextRecorder(recorder);
		recorder = abort_recorder;
	}

	if (config_domain.hs_delay_monitor)
	{
		TcpRecorderSP delay_recorder(new HsDelayRecorder(logger_, config_domain.hs_delay_min));
		delay_recorder->SetNextRecorder(recorder);
		recorder = delay_recorder;
	}

	if (config_domain.hs_timeout_monitor)
	{
		uint32 handshake_timeout;
		GET_TMAS_CONFIG_INT("global.tcp.handshake-timeout", handshake_timeout);

		TcpRecorderSP timeout_recorder(new HsTimeoutRecorder(logger_, handshake_timeout));
		timeout_recorder->SetNextRecorder(recorder);
		recorder = timeout_recorder;
	}

	if (config_domain.conn_timeout_monitor)
	{
		uint32 conn_timeout;
		GET_TMAS_CONFIG_INT("global.tcp.connection-timeout", conn_timeout);

		TcpRecorderSP timeout_recorder(new ConnTimeoutRecorder(logger_, conn_timeout));
		timeout_recorder->SetNextRecorder(recorder);
		recorder = timeout_recorder;
	}

	return recorder;
}

/*------------------------------------------------------------------------------
 * 描  述: 处理连接信息
 * 参  数: [in] conn_info 连接信息
 * 返回值:
 * 修  改:
 *   时间 2014年05月08日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void TcpObserverManager::Process(const TcpRecordInfo& record_info)
{
	for (TcpObserver& observer : observers_)
	{
		observer.Process(record_info);
	}
}

}