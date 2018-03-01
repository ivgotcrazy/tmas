/*#############################################################################
 * �ļ���   : tcp_observer_manager.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : �۲������������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: TCP�۲��ߣ��������ü�¼���ݡ�
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: TCP�۲��߹�������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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