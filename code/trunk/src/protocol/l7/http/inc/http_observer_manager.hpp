/*#############################################################################
 * �ļ���   : http_observer_manager.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��05��08��
 * �ļ����� : HTTP�۲������������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ����: HTTP�۲��ߣ��������ü�¼���ݡ�
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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
 * ����: HTTP�۲��߹�������
 * ���ߣ�teck_zhou
 * ʱ�䣺2014��05��08��
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