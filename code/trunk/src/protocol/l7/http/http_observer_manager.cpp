/*#############################################################################
 * �ļ���   : http_observer_manager.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : HTTP�۲��������ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_observer_manager.hpp"
#include "http_config_parser.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] filter ������
 *         [in] recorder ��¼��
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpObserver::HttpObserver(const HttpFilterSP& filter, const HttpRecorderSP& recorder)
	: filter_(filter), recorder_(recorder)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ���ƥ������������¼�����Ϣ
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ���캯��
 * ��  ��: [in] logger ��¼��
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpObserverManager::HttpObserverManager(const LoggerSP& logger) 
	: logger_(logger)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �������ù���HTTP������
 * ��  ��: [in] config_domain ��������Ϣ 
 * ����ֵ: ������
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpFilterSP HttpObserverManager::ConstructHttpFilter(const HttpConfigDomain& config_domain)
{
	HttpFilterSP filter;

	// ����IP��ַ������
	if (!config_domain.ip_regions.empty())
	{
		HttpFilterSP ip_fileter(new HttpIpFilter(config_domain.ip_regions));
		ip_fileter->SetDecorator(filter);
		filter = ip_fileter;
	}

	// �����˿ڹ�����
	if (!config_domain.port_regions.empty())
	{
		HttpFilterSP port_filter(new HttpPortFilter(config_domain.port_regions));
		port_filter->SetDecorator(filter);
		filter = port_filter;
	}

	// ����Host������
	if (!config_domain.host_regex.empty())
	{
		HttpFilterSP host_filter(new HttpHostFilter(config_domain.host_regex));
		host_filter->SetDecorator(filter);
		filter = host_filter;
	}

	// ����URI������
	if (!config_domain.uri_regex.empty())
	{
		HttpFilterSP uri_filter(new HttpUriFilter(config_domain.uri_regex));
		uri_filter->SetDecorator(filter);
		filter = uri_filter;
	}

	return filter;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����������Ϣ������¼��
 * ��  ��: [in] config_domain ��������Ϣ
 * ����ֵ: ��¼��
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �����¼��Ϣ
 * ��  ��: [in] record_info ��¼��Ϣ
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��05��09��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpObserverManager::Process(const HttpRecordInfo& record_info)
{
	for (HttpObserver& observer : observers_)
	{
		observer.Process(record_info);
	}
}

}