/*#############################################################################
 * �ļ���   : http_recombinder.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��03��
 * �ļ����� : HttpRecombinder��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "http_recombinder.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] handler ����ص�����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
HttpRecombinder::HttpRecombinder(RecombindHandler handler)
	: transfer_type_(HTT_UNKNOWN)
	, cl_recombinder_(handler)
	, chunked_recombinder_(handler)
{
}

/*------------------------------------------------------------------------------
 * ��  ��: ���ô�������Ϊ"content-length"
 * ��  ��: [in] content-length ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRecombinder::SetClTransferType(uint32 content_length)
{
	transfer_type_ = HTT_CONTENT_LENGTH;

	cl_recombinder_.Init(content_length);
}

/*------------------------------------------------------------------------------
 * ��  ��: ���ô�������Ϊ"content-length"
 * ��  ��: [in] content-length ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRecombinder::SetChunkedTransferType()
{
	transfer_type_ = HTT_CHUNKED;

	chunked_recombinder_.Init();
}

/*------------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: [in] data ��������
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void HttpRecombinder::AppendData(const char* data, uint32 len)
{
	if (transfer_type_ == HTT_UNKNOWN)
	{
		TMAS_ASSERT(false);
		return;
	}

	if (transfer_type_ == HTT_CONTENT_LENGTH)
	{
		cl_recombinder_.AppendData(data, len);
	}
	else 
	{
		chunked_recombinder_.AppendData(data, len);
	}
}

}









