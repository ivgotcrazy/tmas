/*#############################################################################
 * �ļ���   : http_recorder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��30��
 * �ļ����� : HttpRecorder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RECORDER
#define BROADINTER_HTTP_RECORDER

#include <vector>
#include "tmas_typedef.hpp"

namespace BroadInter
{

using std::string;

/*******************************************************************************
 * ��  ��: ��¼������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��31��
 ******************************************************************************/
class HttpRecorder
{
public:
	virtual ~HttpRecorder() {}

	virtual void OnTick() = 0;
	virtual void RecordHttpRunSession(const HttpRunSessionSP& run_session) = 0;
};

typedef std::vector<HttpRecorder*> RecorderVec;

/*******************************************************************************
 * ��  ��: URI��¼��
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��31��
 ******************************************************************************/
class UriRecorder : public HttpRecorder
{
public:
	UriRecorder(const string& uri, bool delay, bool speed, uint32 interval);

	virtual void OnTick();
	virtual void RecordHttpRunSession(const HttpRunSessionSP& run_session) override;

private:
	struct HttpUriStat
	{
		HttpUriStat() : total_session_num(0), total_download_size(0)
			, response_delay(0), response_time(0) {}

		uint64 total_session_num;		// ���ӵĸ���
		uint64 total_download_size;		// �����ش�С
		uint64 response_delay;			// ��Ӧʱ��
		uint64 response_time;			// ������Ӧ����ʱ��
	};

private:
	void LogUriStat();
	void ReInit();

private:
	string uri_;

	HttpUriStat uri_stat_;

	bool delay_monitor_;	// ����ʱ�Ӽ�⿪��
	bool speed_monitor_;	// �����ٶȼ�⿪��
	uint32 stat_interval_;	// ͳ��ʱ����

	uint32 elapsed_tick_;	// �Ѿ����ŵ�����
};

/*******************************************************************************
 * ��  ��: Host����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��31��
 ******************************************************************************/
class HostRecorder : public HttpRecorder
{
public:
	HostRecorder(const string& host, bool delay, bool speed, uint32 interval);

	virtual void OnTick();
	virtual void RecordHttpRunSession(const HttpRunSessionSP& run_session) override;

private:
	struct HttpHostStat // ָ��domain��ͳ������
	{
		HttpHostStat() : total_session_num(0), total_download_size(0)
			, response_delay(0), response_time(0) {}

		uint64 total_session_num;	// ���ӵĸ���
		uint64 total_download_size;	// �����ش�С
		uint64 response_delay;		// ��Ӧʱ��
		uint64 response_time;		// ������Ӧ����ʱ��
	};

private:
	void LogHostStat();
	void ReInit();

private:
	string host_;

	HttpHostStat host_stat_;

	bool delay_monitor_;	// ����ʱ�Ӽ�⿪��
	bool speed_monitor_;	// �����ٶȼ�⿪��

	uint32 stat_interval_;	// ͳ��ʱ����

	uint32 elpased_tick_;	// �Ѿ����ŵ�����
};

}

#endif
