/*#############################################################################
 * �ļ���   : http_run_session.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��02��
 * �ļ����� : HttpRunSession��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "http_run_session.hpp"
#include "tmas_assert.hpp"
#include "http_monitor.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"
#include "message.hpp"
#include "http_parser.hpp"

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
HttpRunSession::HttpRunSession(HttpMonitorType* monitor, const ConnId& conn_id)
	: http_monitor_(monitor)
	, session_state_(RSS_SESSION_INIT)
	, session_info_(conn_id)
	, recombinder_(this, false)
{
}

/*------------------------------------------------------------------------------
 * ��  ��: ����״̬�ı��Ĵ���
 * ��  ��: [in] pkt_msg ������Ϣ
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��29��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::ProcessPacket(const PktMsgSP& pkt_msg)
{
	switch (session_state_)
	{
	case RSS_SESSION_INIT:
		SessionInitProc(pkt_msg);
		break;

	case RSS_RECVING_REQUEST_HEADER:
		RecvingRequestHeaderProc(pkt_msg);
		break;

	case RSS_RECVING_REQUEST_DATA:
		RecvingRequestDataProc(pkt_msg);
		break;

	case RSS_RECVED_REQUEST:
		RecvedRequestProc(pkt_msg);
		break;

	case RSS_RECVING_RESPONSE_HEADER:
		RecvingResponseHeaderProc(pkt_msg);
		break;

	case RSS_RECVING_RESPONSE_DATA:
		RecvingResponseDataProc(pkt_msg);
		break;

	case RSS_RECVED_RESPONSE:
		RecvedResponseProc(pkt_msg);
		break;

	default:
		TMAS_ASSERT(false);
		LOG(ERROR) << "Unexpected state " << session_state_;
		return;
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_SESSION_INIT״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::SessionInitProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::SessionInitProc";

	const char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	if (IsCompleteHttpRequest(data, len))
	{
		RecvedCompleteHttpRequest(data, len);
	}
	else if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteRequestHeader(data, len);
	}
	else
	{
		session_info_.request.assign(data, len);
		session_state_ = RSS_RECVING_REQUEST_HEADER;
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_RECVING_REQUEST_HEADER״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingRequestHeaderProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingRequestHeaderProc";

	TMAS_ASSERT(!session_info_.request.empty());

	session_info_.request.append(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));

	const char* data = session_info_.request.c_str();
	uint32 len = session_info_.request.length();

	if (IsCompleteHttpRequest(data, len))
	{
		RecvedCompleteHttpRequest(data, len);
	}
	else if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteRequestHeader(data, len);
	}
	else
	{
		; // ��������
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_RECVING_REQUEST_DATA״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingRequestDataProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingRequestHeaderProc";

	recombinder_.AppendData(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_RECVED_REQUEST״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedRequestProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvedRequestProc";

	const char* data = L7_DATA(pkt_msg);
	uint32 len = L4_DATA_LEN(pkt_msg);

	if (!IsHttpResponse(data, len))
	{
		DLOG(WARNING) << "Should come http response";
		http_monitor_->SessionFailed(shared_from_this(), SFR_UNEXPECTED_PKT);
		return;
	}

	// ��ʼ����HTTP��Ӧ
	session_info_.response_begin = GetMicroSecond();

	if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteResponseHeader(data, len);
	}
	else
	{
		session_info_.response.assign(data, len);
		session_state_ = RSS_RECVING_RESPONSE_HEADER;
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_RECVING_RESPONSE_HEADER״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingResponseHeaderProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingResponseHeaderProc";

	TMAS_ASSERT(!session_info_.response.empty());

	session_info_.response.append(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));

	const char* data = session_info_.response.c_str();
	uint32 len = session_info_.response.length();

	if (HasCompleteHttpHeader(data, len))
	{
		RecvedCompleteResponseHeader(data, len);
	}
	else
	{
		; // ��������
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_RECVING_RESPONSE_DATA״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvingResponseDataProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvingResponseDataProc";

	recombinder_.AppendData(L7_DATA(pkt_msg), L4_DATA_LEN(pkt_msg));
}

/*------------------------------------------------------------------------------
 * ��  ��: RSS_RECVED_RESPONSE״̬����
 * ��  ��: [in] pkt_msg
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedResponseProc(const PktMsgSP& pkt_msg)
{
	DLOG(INFO) << "Enter HttpRunSession::RecvedResponseProc";

	TMAS_ASSERT(false);
}

/*------------------------------------------------------------------------------
 * ��  ��: ���յ�����HTTP������
 * ��  ��: [in] data ��������
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteHttpRequest(const char* data, uint32 len)
{
	TMAS_ASSERT((session_state_ == RSS_SESSION_INIT)
		|| (session_state_ == RSS_RECVING_REQUEST_HEADER));

	HttpRequest http_request;
	if (!HPN::ParseHttpRequest(data, len, http_request))
	{
		DLOG(WARNING) << "Fail to parse http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_PARSE_REQUEST_ERR);
		return;
	}

	session_info_.uri = http_request.request_line.uri;

	HttpFilterManager& filter_manager = http_monitor_->GetHttpFilterManager();
	filter_manager.GetMatchedRecorders(http_request, session_info_.conn_id.smaller_ip,
										session_info_.conn_id.bigger_ip, recorders_);
	if (recorders_.empty())
	{
		DLOG(WARNING) << "Not interest http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_NOT_INTEREST_REQUEST);
		return;
	}

	session_info_.request.clear();

	session_info_.request_end = GetMicroSecond();

	session_info_.request_size = len;

	session_state_ = RSS_RECVED_REQUEST;
}

/*------------------------------------------------------------------------------
 * ��  ��: ���յ�����HTTP����ͷ����
 * ��  ��: [in] data ��������
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��01��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRunSession::RecvedCompleteRequestHeader(const char* data, uint32 len)
{
	TMAS_ASSERT((session_state_ == RSS_SESSION_INIT)
		|| (session_state_ == RSS_RECVING_REQUEST_HEADER));

	HttpRequest http_request;
	if (!HPN::ParseHttpRequest(data, len, http_request))
	{
		DLOG(ERROR) << "Fail to parse http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_PARSE_REQUEST_ERR);
		return;
	}

	session_info_.uri = http_request.request_line.uri;

	HttpFilterManager& filter_manager = http_monitor_->GetHttpFilterManager();
	filter_manager.GetMatchedRecorders(http_request, session_info_.conn_id.smaller_ip,
										session_info_.conn_id.bigger_ip, recorders_);
	if (recorders_.empty())
	{
		DLOG(WARNING) << "Not interest http request";
		http_monitor_->SessionFailed(shared_from_this(), SFR_NOT_INTEREST_REQUEST);
		return;
	}

	session_info_.request.clear();

	session_state_ = RSS_RECVING_REQUEST_DATA;

	SetTransferType(http_request.header);

	if (http_request.body.len > 0)
	{
		recombinder_.AppendData(http_request.body.start, http_request.body.len);
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
void HttpRunSession::RecvedCompleteResponseHeader(const char* data, uint32 len)
{
	TMAS_ASSERT((session_state_ == RSS_RECVING_RESPONSE_HEADER)
		|| (session_state_ == RSS_RECVED_REQUEST));

	HttpResponse http_response;
	if (!HPN::ParseHttpResponse(data, len, http_response))
	{
		DLOG(ERROR) << "Fail to parse http response";
		http_monitor_->SessionFailed(shared_from_this(), SFR_PARSE_RESPONSE_ERR);
		return;
	}

	SetTransferType(http_response.header);

	session_state_ = RSS_RECVING_RESPONSE_DATA;

	// �������ݳ����Ƿ�Ϊ0������Ҫ���ݵ�recombinder��������ȷ����Ӧ�Ƿ����
	recombinder_.AppendData(http_response.body.start, http_response.body.len);
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
	// ����HTTP��Ӧ����
	session_info_.response_end = GetMicroSecond();

	if (!result)
	{
		http_monitor_->SessionFailed(shared_from_this(), SFR_RECOMBIND_ERR);
		return;
	}

	if (session_state_ == RSS_RECVING_REQUEST_DATA)
	{
		RecvedCompleteRequestData(data, len);
	}
	else if (session_state_ == RSS_RECVING_RESPONSE_DATA)
	{
		RecvedCompleteResponseData(data, len);
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
void HttpRunSession::RecvedCompleteRequestData(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_REQUEST;

	data = data; // for compile warning

	session_info_.request_end = GetMicroSecond();
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
void HttpRunSession::RecvedCompleteResponseData(const char* data, uint32 len)
{
	session_state_ = RSS_RECVED_RESPONSE;

	data = data; // for compile warning

	session_info_.response_size = len;

	http_monitor_->SessionCompleted(shared_from_this());
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

	TMAS_ASSERT(!(cl_iter != http_header.end() && te_iter != http_header.end()));

	if (cl_iter == http_header.end() && te_iter == http_header.end())
	{
		recombinder_.SetUnknownTransferType();
	}
	else if (te_iter != http_header.end())
	{
		recombinder_.SetChunkedTransferType();
	}
	else
	{
		uint32 length = 0;
		try
		{
			length = boost::lexical_cast<uint32>(cl_iter->second);
		}
		catch (...)
		{
			LOG(ERROR) << "Invalid content-length : " << cl_iter->second;
		}

		recombinder_.SetClTransferType(length);
	}
}

}