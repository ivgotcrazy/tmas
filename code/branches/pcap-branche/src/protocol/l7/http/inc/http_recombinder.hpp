/*#############################################################################
 * 文件名   : http_recombinder.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月01日
 * 文件描述 : HttpRecombinder类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * 描  述: HTTP报文重组器
 * 作  者: teck_zhou
 * 时  间: 2014年03月03日
 ******************************************************************************/
class HttpRecombinder : public boost::noncopyable
{
public:
	HttpRecombinder(HttpRunSession* run_session, bool copy_data);

	void AppendData(const char* data, uint32 len);

	void SetUnknownTransferType();

	// 设置解析方式为"content-length"
	void SetClTransferType(uint32 content_len);

	// 设置解析方式为"transfer-encoding: chunked"
	void SetChunkedTransferType();

private:
	HttpTransferType transfer_type_;

	UnknownRecombinder unknown_recombinder_;
	ContentLengthRecombinder cl_recombinder_;
	ChunkedRecombinder chunked_recombinder_;
};

}

#endif