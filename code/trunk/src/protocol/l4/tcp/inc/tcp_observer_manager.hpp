/*#############################################################################
 * 文件名   : tcp_observer_manager.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年05月08日
 * 文件描述 : 观察器相关类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TCP_OBSERVER_MANAGER
#define BROADINTER_TCP_OBSERVER_MANAGER

#include <vector>
#include <boost/noncopyable.hpp>

#include "tcp_filter.hpp"
#include "tcp_recorder.hpp"
#include "tcp_typedef.hpp"
#include "tcp_config_parser.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: TCP观察者，根据配置记录数据。
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpObserver
{
public:
	TcpObserver(const TcpFilterSP& filter, const TcpRecorderSP& recorder);

	void Process(const TcpRecordInfo& record_info);

private:
	TcpFilterSP filter_;
	TcpRecorderSP recorder_;
};

/******************************************************************************
 * 描述: TCP观察者管理容器
 * 作者：teck_zhou
 * 时间：2014年05月08日
 *****************************************************************************/
class TcpObserverManager : public boost::noncopyable
{
public:
	TcpObserverManager(const LoggerSP& logger);

	bool Init();
	void Process(const TcpRecordInfo& record_info);

private:
	TcpFilterSP ConstructTcpFilter(const TcpConfigDomain& config_domain);
	TcpRecorderSP ConstructTcpRecorder(const TcpConfigDomain& config_domain);

private:
	std::vector<TcpObserver> observers_;
	LoggerSP logger_;
};

}

#endif