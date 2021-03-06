/*#############################################################################
 * 文件名   : http_recombinder.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月03日
 * 文件描述 : HttpRecombinder类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include <boost/bind.hpp>

#include "http_recombinder.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*------------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] handler 重组回调函数
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
HttpRecombinder::HttpRecombinder(HttpRunSession* run_session, bool copy_data)
	: transfer_type_(HTT_UNKNOWN)
	, unknown_recombinder_(run_session)
	, cl_recombinder_(run_session, copy_data)
	, chunked_recombinder_(run_session, copy_data)
{
}

/*------------------------------------------------------------------------------
 * 描  述: 设置传输类型为Unknown
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年04月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRecombinder::SetUnknownTransferType()
{
	transfer_type_ = HTT_UNKNOWN;
}

/*------------------------------------------------------------------------------
 * 描  述: 设置传输类型为"content-length"
 * 参  数: [in] content-length 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRecombinder::SetClTransferType(uint32 content_length)
{
	transfer_type_ = HTT_CONTENT_LENGTH;

	cl_recombinder_.Init(content_length);
}

/*------------------------------------------------------------------------------
 * 描  述: 设置传输类型为"content-length"
 * 参  数: [in] content-length 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRecombinder::SetChunkedTransferType()
{
	transfer_type_ = HTT_CHUNKED;

	chunked_recombinder_.Init();
}

/*------------------------------------------------------------------------------
 * 描  述: 附加数据
 * 参  数: [in] data 报文数据
 *         [in] len 数据长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月03日
 *   作者 teck_zhou
 *   描述 创建
 -----------------------------------------------------------------------------*/
void HttpRecombinder::AppendData(const char* data, uint32 len)
{
	if (transfer_type_ == HTT_CONTENT_LENGTH)
	{
		cl_recombinder_.AppendData(data, len);
	}
	else if (transfer_type_ == HTT_CHUNKED)
	{
		chunked_recombinder_.AppendData(data, len);
	}
	else
	{
		unknown_recombinder_.AppendData(data, len);
	}
}

}

