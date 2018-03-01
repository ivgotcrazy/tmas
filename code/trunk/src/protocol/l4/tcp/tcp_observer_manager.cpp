/*#############################################################################
 * �ļ���   : tcp_observer_manager.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : �۲��������ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "tcp_observer_manager.hpp"
#include "tmas_config_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] filter ������
 *         [in] recorder ��¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
TcpObserver::TcpObserver(const TcpFilterSP& filter, const TcpRecorderSP& recorder)
	: filter_(filter), recorder_(recorder)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ����������Ϣ(�ϻ�)
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
TcpObserverManager::TcpObserverManager(const LoggerSP& logger) : logger_(logger)
{
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʼ��(�ϻ�)
 * ��  ��: 
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����������
 * ��  ��: [in] config_domain ��������Ϣ
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
TcpFilterSP TcpObserverManager::ConstructTcpFilter(const TcpConfigDomain& config_domain)
{
	TcpFilterSP filter;

	// ����IP��ַ������
	if (!config_domain.ip_regions.empty())
	{
		TcpFilterSP ip_fileter(new TcpIpFilter(config_domain.ip_regions));
		ip_fileter->SetDecorator(filter);
		filter = ip_fileter;
	}

	// �����˿ڹ�����
	if (!config_domain.port_regions.empty())
	{
		TcpFilterSP port_filter(new TcpPortFilter(config_domain.port_regions));
		port_filter->SetDecorator(filter);
		filter = port_filter;
	}

	return filter;
}

/*------------------------------------------------------------------------------
 * ��  ��: ������¼��
 * ��  ��: [in] config_domain ��������Ϣ
 * ����ֵ: ��¼��
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����������Ϣ
 * ��  ��: [in] conn_info ������Ϣ
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��05��08��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void TcpObserverManager::Process(const TcpRecordInfo& record_info)
{
	for (TcpObserver& observer : observers_)
	{
		observer.Process(record_info);
	}
}

}