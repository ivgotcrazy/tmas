/*#############################################################################
 * �ļ���   : http_recorder.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��31��
 * �ļ����� : HttpRecorder��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>
#include <fstream>

#include "http_recorder.hpp"
#include "http_typedef.hpp"
#include "tmas_util.hpp"
#include "tmas_assert.hpp"
#include "http_run_session.hpp"

namespace BroadInter
{
/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] uri URI������ʽ
 *         [in] delay ʱ�Ӽ�⿪��
 *         [in] speed �����ٶȼ�⿪��
 *         [in] interval ��¼ʱ����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
UriRecorder::UriRecorder(const string& uri, bool delay, bool speed, uint32 interval)
	: uri_(uri)
	, delay_monitor_(delay)
	, speed_monitor_(speed)
	, stat_interval_(interval)
	, elapsed_tick_(0)
{

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
void UriRecorder::OnTick()
{
	if (stat_interval_ == 0) return;

	if (++elapsed_tick_ >= stat_interval_)
	{
		LogUriStat();
		ReInit();
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼URIͳ����Ϣ
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void UriRecorder::LogUriStat()
{
#if 0
	std::ofstream ofs("uri_" + uri_ + ".log", std::ios_base::app);
	if (!ofs)
	{
		LOG(ERROR) << "Fail to create uri log file | " << "uri_" + uri_ + ".log";
		return;
	}

	// �����ʽ��time session_num response_delay response_speed

	if (uri_stat_.total_session_num == 0)
	{
		ofs << GetLocalTime() << " " 
			<< std::setw(5) << 0 << " " 
			<< std::setw(10) << 0 << " " 
			<< std::setw(10) << 0 << std::endl;
	}
	else
	{
		TMAS_ASSERT(uri_stat_.response_delay != 0);
		TMAS_ASSERT(uri_stat_.response_time != 0);

		ofs << GetLocalTime() << " " 
			<< std::setw(5) << uri_stat_.total_session_num << " "
			<< std::setw(10) << uri_stat_.response_delay / uri_stat_.total_session_num << " "
			<< std::setw(10) << uri_stat_.total_download_size * 1000 * 1000 / uri_stat_.response_time
			<< std::endl;
	}

	ofs.close();
#endif
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���³�ʼ��ͳ������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void UriRecorder::ReInit()
{
	elapsed_tick_ = 0;

	std::memset(&uri_stat_, 0x0, sizeof(uri_stat_));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼RunSession
 * ��  ��: [in] run_session HttpRunSession
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void UriRecorder::RecordHttpRunSession(const HttpRunSessionSP& run_session)
{
	uri_stat_.total_session_num++;

	const RunSessionInfo& info = run_session->GetRunSessionInfo();

	uri_stat_.total_download_size += info.response_size;

	//TMAS_ASSERT(info.response_begin > info.request_end);

	uri_stat_.response_delay += (info.response_begin - info.request_end);

	//TMAS_ASSERT(info.response_end > info.response_begin);

	uri_stat_.response_time += (info.response_end - info.request_end);
	
	if (stat_interval_ == 0)
	{
		LogUriStat();
		ReInit();
	}
}

////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] host Host������ʽ
 *         [in] delay ʱ�Ӽ�⿪��
 *         [in] speed �����ٶȼ�⿪��
 *         [in] interval ��¼ʱ����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
HostRecorder::HostRecorder(const string& host, bool delay, bool speed, uint32 interval)
	: host_(host)
	, delay_monitor_(delay)
	, speed_monitor_(speed)
	, stat_interval_(interval)
	, elpased_tick_(0)
{

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
void HostRecorder::OnTick()
{
	if (stat_interval_ == 0) return;

	if (++elpased_tick_ >= stat_interval_)
	{
		LogHostStat();
		ReInit();	
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼Hostͳ����Ϣ
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HostRecorder::LogHostStat()
{
#if 0
	std::ofstream ofs("host_" + host_ + ".log", std::ios_base::app);
	if (!ofs)
	{
		LOG(ERROR) << "Fail to create host log file | " << "host_" + host_ + ".log";
		return;
	}
	
	// �����ʽ��time | session_num | response_delay | response_speed

	if (host_stat_.total_session_num == 0)
	{
		ofs << GetLocalTime() << " " 
			<< std::setw(5) << 0 << " " 
			<< std::setw(10) << 0 << " " 
			<< std::setw(10) << 0 << std::endl;
	}
	else
	{
		TMAS_ASSERT(host_stat_.response_delay != 0);
		TMAS_ASSERT(host_stat_.response_time != 0);

		ofs << GetLocalTime() << " "
			<< std::setw(5) << host_stat_.total_session_num << " "
			<< std::setw(10) << host_stat_.response_delay / host_stat_.total_session_num << " "
			<< std::setw(10) << host_stat_.total_download_size * 1000 * 1000 / host_stat_.response_time
			<< std::endl;
	}

	ofs.close();
#endif
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���³�ʼ��ͳ������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HostRecorder::ReInit()
{
	elpased_tick_ = 0;

	std::memset(&host_stat_, 0x0, sizeof(host_stat_));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��¼һ��RunSession��Ϣ
 * ��  ��: [in] run_session HttpRunSession
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��31��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void HostRecorder::RecordHttpRunSession(const HttpRunSessionSP& run_session)
{
	host_stat_.total_session_num++;

	const RunSessionInfo& info = run_session->GetRunSessionInfo();

	host_stat_.total_download_size += info.response_size;

	TMAS_ASSERT(info.response_begin > info.request_end);

	host_stat_.response_delay += (info.response_begin - info.request_end);

	TMAS_ASSERT(info.response_end > info.request_end);

	host_stat_.response_time += (info.response_end - info.request_end);

	if (stat_interval_ == 0)
	{
		LogHostStat();
		ReInit();
	}
}

}
