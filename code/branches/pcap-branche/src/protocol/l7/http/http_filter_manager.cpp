/*#############################################################################
 * �ļ���   : http_filter_manager.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��31��
 * �ļ����� : HttpFilterManager��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_filter_manager.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] uri URI������ʽ
 *         [in] recoder ��Ӧ�ļ�¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
UriFilter::UriFilter(const string& uri, const IpAddrVec& src_ip, const IpAddrVec& dst_ip, HttpRecorder* recorder)
	: HttpFilter(recorder)
	, uri_regex_(boost::regex(uri))
	, src_ips_(src_ip)
	, dst_ips_(dst_ip)
{

}

/*-----------------------------------------------------------------------------
 * ��  ��: ƥ��HTTP����
 * ��  ��: [in] request HTTP����
 * ����ֵ: ƥ����:�ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool UriFilter::MatchHttpRequest(const HttpRequest& request,
										const uint32 smaller_ip, const uint32 bigger_ip)
{
	if (!boost::regex_match(request.request_line.uri, uri_regex_)) return false;

	//����Ŀǰ��֧��ԭ��ַ��Ŀ���ַ������ֻҪ��ַ��ƥ��ɹ�����
	if ((!MatchIpRegion(smaller_ip, bigger_ip)) && (!MatchIpRegion(bigger_ip, smaller_ip)))
	{
		DLOG(INFO) << "Match uri ip region failed";
		return false;
	}	

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ƥ��Ip��ַ��
 * ��  ��: [in] src_addr Դ��ַ
 *         [in] dst_addr Ŀ���ַ
 * ����ֵ: ƥ����:�ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��17��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool UriFilter::MatchIpRegion(const uint32 src_addr, const uint32 dst_addr)
{
	bool ret = false;
	for (auto src_region : src_ips_)
	{
		if (src_region.start_addr <= src_addr &&  src_addr <= src_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth uri src ip region failed";
		return false;
	}

	ret = false;
	for (auto dst_region : dst_ips_)
	{
		if (dst_region.start_addr <= dst_addr &&  dst_addr <= dst_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth uri dst ip region failed";
		return false;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] host Host������ʽ
 *         [in] uris ������URI����
 *         [in] recoder ��Ӧ�ļ�¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
HostFilter::HostFilter(const string& host, const UriVec& uris, const IpAddrVec& src_ip, 
							const IpAddrVec& dst_ip, HttpRecorder* recorder)
	: HttpFilter(recorder)
	, host_regex_(boost::regex(host))
	, src_ips_(src_ip)
	, dst_ips_(dst_ip)
{
	for (const string& uri : uris)
	{
		uri_regexs_.push_back(boost::regex(uri));
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ƥ��Ip��ַ��
 * ��  ��: [in] src_addr Դ��ַ
 *         [in] dst_addr Ŀ���ַ
 * ����ֵ: ƥ����:�ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��17��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HostFilter::MatchHttpRequest(const HttpRequest& request, const uint32 smaller_ip, 
											const uint32 bigger_ip)
{
	// �������ͷ��û��"host"�ֶΣ�ƥ��ʧ��
	auto host_iter = request.header.find("host");
	if (host_iter == request.header.end())
	{
		DLOG(WARNING) << "Can not find host in http request";
		return false;
	}

	bool ret = false;
	if (boost::regex_match(host_iter->second, host_regex_))
	{
		if (uri_regexs_.empty())
		{
			ret = true;
		}
		else
		{
			for (const boost::regex& uri_regex : uri_regexs_)
			{
				if (boost::regex_match(request.request_line.uri, uri_regex))
				{
					ret = true;
					break;
				}
			}
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Match Host or uri failed";
		return false;
	}
	
	//����Ŀǰ��֧��ԭ��ַ��Ŀ���ַ������ֻҪ��ַ��ƥ��ɹ�����
	if ((!MatchIpRegion(smaller_ip, bigger_ip)) && (!MatchIpRegion(bigger_ip, smaller_ip)))
	{
		DLOG(INFO) << "Match host ip region failed";
		return false;
	}
	
	return ret;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ƥ��Ip��ַ��
 * ��  ��: [in] request HTTP����
 * ����ֵ: ƥ����:�ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��17��
 *   ���� tom_liu
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HostFilter::MatchIpRegion(const uint32 src_addr, const uint32 dst_addr)
{	
	bool ret = false;
	for (auto src_region : src_ips_)
	{
		if (src_region.start_addr <= src_addr &&  src_addr <= src_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth host src ip region failed";
		return false;
	}

	ret = false;
	for (auto dst_region : dst_ips_)
	{
		if (dst_region.start_addr <= dst_addr &&  dst_addr <= dst_region.end_addr)
		{
			ret = true;
			break;
		}
	}

	if (!ret)
	{
		DLOG(INFO) << "Macth host dst ip region failed";
		return false;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
HttpFilterManager::~HttpFilterManager()
{
	record_timer_->Stop();

	for (HostFilter* filter : host_filters_)
	{
		delete filter;
	}

	for (UriFilter* filter : uri_filters_)
	{
		delete filter;
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��:
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HttpFilterManager::Init()
{
	HostConfigVec host_configs;
	UriConfigVec uri_configs;

	if (!HttpConfigParser::ParseConfig(host_configs, uri_configs))
	{
		LOG(ERROR) << "Fail to parse http config";
		return false;
	}

	for (const HostConfig& host_config : host_configs)
	{
		HostRecorder* recorder = 
			new HostRecorder(host_config.host, host_config.delay_monitor, 
				host_config.speed_monitor, host_config.stat_interval);

		host_filters_.push_back(
			new HostFilter(host_config.host, host_config.uris, 
							host_config.src_ips, host_config.dst_ips, recorder));
	}

	for (const UriConfig& uri_config : uri_configs)
	{
		UriRecorder* recorder = 
			new UriRecorder(uri_config.uri, uri_config.delay_monitor, 
				uri_config.speed_monitor, uri_config.stat_interval);

		uri_filters_.push_back(new UriFilter(uri_config.uri,
								uri_config.src_ips, uri_config.dst_ips, recorder));
	}

	record_timer_.reset(new FreeTimer(
		boost::bind(&HttpFilterManager::OnTick, this), 1));

	record_timer_->Start();

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ʱ������
 * ��  ��:
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HttpFilterManager::OnTick()
{
	for (HostFilter* filter : host_filters_)
	{
		filter->GetHttpRecorder()->OnTick();
	}

	for (UriFilter* filter : uri_filters_)
	{
		filter->GetHttpRecorder()->OnTick();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ�����Ӧ�Ĺ�����
 * ��  ��: [in] request HTTP����
 *         [out] recorders ��ؼ�¼��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HttpFilterManager::GetMatchedRecorders(const HttpRequest& request,
													const uint32 smaller_ip, 
													const uint32 bigger_ip,
													RecorderVec	& recorders)
{
	// ��ƥ��URI����
	for (UriFilter* filter : uri_filters_)
	{
		if (filter->MatchHttpRequest(request, smaller_ip, bigger_ip))
		{
			recorders.push_back(filter->GetHttpRecorder());
		}
	}

	// ��ƥ��Host����
	for (HostFilter* filter : host_filters_)
	{
		if (filter->MatchHttpRequest(request, smaller_ip, bigger_ip))
		{
			recorders.push_back(filter->GetHttpRecorder());
		}
	}
}

}