/*#############################################################################
 * �ļ���   : http_run_session.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��02��
 * �ļ����� : HttpRunSession������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RUN_SESSION
#define BROADINTER_HTTP_RUN_SESSION

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include "message.hpp"
#include "http_typedef.hpp"
#include "http_recombinder.hpp"
#include "tmas_typedef.hpp"
#include "http_recorder.hpp"

namespace BroadInter
{

using std::string;

class HttpRecorder;

/*******************************************************************************
 * ��  ��: HTTP����ʱsession
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��02��
 ******************************************************************************/
class HttpRunSession : public boost::enable_shared_from_this<HttpRunSession>
					 , public boost::noncopyable
{
public:
	HttpRunSession(HttpMonitorType* monitor, const ConnId& conn_id);

	//~HttpRunSession() { LOG(INFO) << "Run session state: " << session_state_; }

	// ���Ĵ������
	void ProcessPacket(const PktMsgSP& pkt_msg);

	// ��������ص�����
	void RecombindCallback(bool result, const char* data, uint32 len);

	RecorderVec& GetRecorders() { return recorders_; }

	const RunSessionInfo& GetRunSessionInfo() const { return session_info_; }

private:
	enum RunSessionState
	{
		RSS_SESSION_INIT,				// ��ʼ״̬
		RSS_RECVING_REQUEST_HEADER,		// ���ڽ�������ͷ
		RSS_RECVING_REQUEST_DATA,		// ���ڽ�����������
		RSS_RECVED_REQUEST,				// �Ѿ�������������
		RSS_RECVING_RESPONSE_HEADER,	// ���ڽ�����Ӧͷ
		RSS_RECVING_RESPONSE_DATA,		// ���ڽ�����Ӧ����
		RSS_RECVED_RESPONSE				// �Ѿ�����������Ӧ
	};

private:
	
	// ����body���ͷ�ʽ(������ʽ)
	void SetTransferType(const HttpHeader& http_header);

	// ״̬������
	void SessionInitProc(const PktMsgSP& pkt_msg);
	void RecvingRequestHeaderProc(const PktMsgSP& pkt_msg);
	void RecvingRequestDataProc(const PktMsgSP& pkt_msg);
	void RecvedRequestProc(const PktMsgSP& pkt_msg);
	void RecvingResponseHeaderProc(const PktMsgSP& pkt_msg);
	void RecvingResponseDataProc(const PktMsgSP& pkt_msg);
	void RecvedResponseProc(const PktMsgSP& pkt_msg);

	// �յ�������HTTP������
	void RecvedCompleteHttpRequest(const char* data, uint32 len);

	// �յ�����HTTP��Ӧͷ����
	void RecvedCompleteResponseHeader(const char* data, uint32 len);

	// �յ�����HTTP����ͷ����
	void RecvedCompleteRequestHeader(const char* data, uint32 len);

	// �յ�����HTTP����body����
	void RecvedCompleteRequestData(const char* data, uint32 len);

	// �յ�����HTTP��Ӧbody����
	void RecvedCompleteResponseData(const char* data, uint32 len);

private:
	HttpMonitorType* http_monitor_;	// ����HttpMonitor

	RunSessionState session_state_;	// �Ự״̬

	RunSessionInfo session_info_;	// �Ự��Ϣ

	HttpRecombinder recombinder_;	// ����������

	RecorderVec recorders_;			// �������ļ�¼��
};

}

#endif