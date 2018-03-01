/*#############################################################################
 * �ļ���   : http_recombinder.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��01��
 * �ļ����� : HttpRecombinder������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RECOMBINDER
#define BROADINTER_HTTP_RECOMBINDER

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "tmas_typedef.hpp"
#include "message.hpp"
#include "http_typedef.hpp"
#include "http_recombinder_impl.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: HTTP����������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��03��
 ******************************************************************************/
class HttpRecombinder : public boost::noncopyable
{
public:
	HttpRecombinder(HttpRunSession* run_session, bool copy_data);

	void AppendData(const char* data, uint32 len);

	void SetUnknownTransferType();

	// ���ý�����ʽΪ"content-length"
	void SetClTransferType(uint32 content_len);

	// ���ý�����ʽΪ"transfer-encoding: chunked"
	void SetChunkedTransferType();

private:
	HttpTransferType transfer_type_;

	UnknownRecombinder unknown_recombinder_;
	ContentLengthRecombinder cl_recombinder_;
	ChunkedRecombinder chunked_recombinder_;
};

}

#endif