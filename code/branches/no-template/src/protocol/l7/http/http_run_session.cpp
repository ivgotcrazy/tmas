/*#############################################################################
 * �ļ���   : http_run_session.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��02��
 * �ļ����� : HttpRunSession������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "http_run_session.hpp"
#include "tmas_assert.hpp"
#include "http_monitor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: ��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpRunSession::HttpRunSession(HttpMonitor* monitor, const ConnId& conn_id,
	const HttpHeader& http_header, const string& domain, const string& uri, 
	const HttpMatchType type, const string& regex_uri)
	: http_monitor_(monitor)
	, recombinder_(boost::bind(&HttpRunSession::RecombindCallback, this, _1, _2, _3))
	, session_state_(RSS_INVALID)
	, session_info_(conn_id, domain, uri, type, regex_uri)
	, match_type_(type)
{
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ�HTTP��������
 * ��  ��: [in] request_line ��������Ϣ
 *         [in] http_header ����ͷ��Ϣ
 *         [in] data ��������
 *         [in] len ���ĳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RcvdRequestPkt(const HttpRequestLine& request_line, 
									const HttpHeader& http_header, 
									const char* data, uint32 len)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	if (session_state_ != RSS_INVALID)
	{
		TMAS_ASSERT(false);
		LOG(ERROR) << "Unexpected session state " << session_state_;
		return;
	}

	SetTransferType(http_header);

	session_state_ = RSS_RECVING_REQUEST;

	recombinder_.AppendData(data, len);
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ�HTTP��Ӧ������
 * ��  ��: [in] http_header HTTPЭ��ͷ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::SetTransferType(const HttpHeader& http_header)
{
	auto cl_iter = http_header.find("content-length");
	auto te_iter = http_header.find("transfer-encoding");

	if (cl_iter != http_header.end() && te_iter != http_header.end())
	{
		LOG(ERROR) << "Invalid request transfer type";
		return;
	}

	if (te_iter != http_header.end())
	{
		recombinder_.SetChunkedTransferType();
	}
	else
	{
		uint32 length = 0;
		if (cl_iter != http_header.end())
		{
			try
			{
				length = boost::lexical_cast<uint32>(cl_iter->second);
			}
			catch (...)
			{
				length = 0;
			}
		}

		recombinder_.SetClTransferType(length);
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ�HTTP��Ӧ������
 * ��  ��: [in] status_line ��Ӧ����Ϣ
 *         [in] http_header ����ͷ��Ϣ
 *         [in] data ��������
 *         [in] len ���ĳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RcvdResponsePkt(const HttpStatusLine& status_line, 
	                                 const HttpHeader& http_header, 
									 const char* data, uint32 len)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	if (session_state_ != RSS_RECVED_REQUEST)
	{
		TMAS_ASSERT(false);
		LOG(ERROR) << "Unexpected session state " << session_state_;
		return;
	}

	SetTransferType(http_header);

	session_state_ = RSS_RECVING_RESPONSE;

	recombinder_.AppendData(data, len);
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ���ʶ���Ĵ�����
 * ��  ��: [in] data ��������
 *         [in] len ���ĳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RcvdUnrecognizedPkt(const char* data, uint32 len)
{
	boost::mutex::scoped_lock lock(run_session_mutex_);

	if (session_state_ != RSS_RECVING_REQUEST 
		&& session_state_ != RSS_RECVING_RESPONSE)
	{
		LOG(ERROR) << "Unexpected session state " << session_state_;
		TMAS_ASSERT(false);
		return;
	}

	recombinder_.AppendData(data, len);
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ�HTTP�������Ĵ�����
 * ��  ��: [in] result �ɹ�/ʧ��
 *         [in] data ����
 *         [in] len ����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecombindCallback(bool result, const char* data, uint32 len)
{
	if (!result)
	{
		http_monitor_->SessionFailed(shared_from_this());
		return;
	}

	if (session_state_ == RSS_RECVING_REQUEST)
	{
		ReceivedCompleteRequest(data, len);
	}
	else if (session_state_ == RSS_RECVING_RESPONSE)
	{
		ReceivedCompleteResponse(data, len);
	}
	else
	{
		TMAS_ASSERT(false);
		LOG(ERROR) << "Invalid session state " << session_state_;
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ�HTTP������
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::ReceivedCompleteRequest(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_REQUEST;

	data = data; // for compile warning

	session_info_.request_time = GetMicroSecond();
	session_info_.request_size = len;
}

/*------------------------------------------------------------------------------
 * ��  ��: �յ�HTTP��Ӧ����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::ReceivedCompleteResponse(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_RESPONSE;

	data = data; // for compile warning

	session_info_.response_time = GetMicroSecond();
	session_info_.response_size = len;

	http_monitor_->SessionCompleted(shared_from_this());
}

}