/*#############################################################################
 * 文件名   : http_parser.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月12日
 * 文件描述 : HttpParser类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>

#include "http_parser.hpp"
#include "tmas_assert.hpp"
#include "tmas_util.hpp"

namespace BroadInter
{

#define CR		'\r'
#define LF		'\n'
#define SPACE	' '
#define COLON	':'

namespace HP
{

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP请求
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] request 请求信息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpRequest(const char* data, uint32 len, HttpRequest& request)
{
	DLOG(INFO) << "Start to parse http request | len: " << len;

	uint32 parse_len = 0;

	char* parse_start = const_cast<char*>(data);
	uint32 data_len = len;

	if (!ParseHttpRequestLine(parse_start, data_len, 
		                      request.request_line, parse_len)
		|| parse_len > data_len)
	{
		LOG(ERROR) << "Fail to parse request line";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http request only has request line";
		return false;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	if (!ParseHttpHeader(parse_start, data_len, request.header, parse_len)
		|| parse_len > data_len)
	{
		LOG(ERROR) << "Fail to parse request header";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http reqeust does not have body";
		return true;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	request.body.start = parse_start;
	request.body.len = data_len;

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP响应
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] request 响应信息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpResponse(const char* data, uint32 len, HttpResponse& response)
{
	uint32 parse_len = 0;

	char* parse_start = const_cast<char*>(data);
	uint32 data_len = len;

	if (!ParseHttpStatusLine(parse_start, data_len, 
		                     response.status_line, parse_len)
		|| parse_len > data_len)
	{
		LOG(ERROR) << "Fail to parse response line";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http response only has status line";
		return false;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	if (!ParseHttpHeader(parse_start, data_len, response.header, parse_len)
		|| parse_len > data_len)
	{
		LOG(ERROR) << "Fail to parse request header";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http response does not have body";
		return false;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	response.body.start = parse_start;
	response.body.len = data_len;

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP请求行
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] request_line 请求行信息
 *         [out] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpRequestLine(const char* data, uint32 len, 
						  HttpRequestLine& request_line, uint32& parse_len)
{
	char* data_begin = const_cast<char*>(data);
	char* data_end = data_begin + len;

	//-- method

	char* delim_pos = FindUntilPos(data_begin, data_end, SPACE);
	if (delim_pos == data_end)
	{
		LOG(ERROR) << "Can not find http request method";
		return false;
	}
	
	request_line.method.assign(data_begin, delim_pos);
	boost::to_upper(request_line.method);
	
	// 从当前空格处开始寻找第一个非空格位置
	data_begin = FindUntilNotPos(delim_pos, data_end, SPACE);
	if (data_begin == data_end)
	{
		LOG(ERROR) << "Can not find not space data";
		return false;
	}

	//-- URI

	delim_pos = FindUntilPos(data_begin, data_end, SPACE);
	if (delim_pos == data_end)
	{
		LOG(ERROR) << "Can not find end delim of uri";
		return false;
	}
	request_line.uri.assign(data_begin, delim_pos);

	// 从当前空格处开始寻找第一个非空格位置
	data_begin = FindUntilNotPos(delim_pos, data_end, SPACE);
	if (data_begin == data_end)
	{
		LOG(ERROR) << "Can not find not space data";
		return false;
	}

	//-- version

	delim_pos = FindUntilPos2(data_begin, data_end, CR, LF);
	if (delim_pos == data_end)
	{
		LOG(ERROR) << "Can not find end delim of http version";
		return false;
	}

	request_line.version.assign(data_begin, delim_pos);
	boost::to_upper(request_line.version);

	if (request_line.version != "HTTP/1.0" 
		&& request_line.version != "HTTP/1.1")
	{
		LOG(ERROR) << "Invalid http version in request";
		return false;
	}

	// skip delim
	data_begin = FindUntilNotPos2(delim_pos, data_end, CR, LF);

	parse_len = data_begin - data;

	TMAS_ASSERT(parse_len <= len);

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP响应行
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] response 响应信息
 *         [out] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpStatusLine(const char* data, uint32 len, 
	                     HttpStatusLine& status_line, uint32& parse_len)
{
	char* data_begin = const_cast<char*>(data);
	char* data_end = data_begin + len;

	//-- version

	char* delim_pos = FindUntilPos(data_begin, data_end, SPACE);
	if (delim_pos == data_end)
	{
		LOG(ERROR) << "Can not find http response version";
		return false;
	}

	status_line.version.assign(data_begin, delim_pos);
	boost::to_upper(status_line.version);

	if (status_line.version != "HTTP/1.0" 
		&& status_line.version != "HTTP/1.1")
	{
		LOG(ERROR) << "Invalid http version in response";
		return false;
	}

	data_begin = FindUntilNotPos(delim_pos, data_end, SPACE);
	if (data_begin == data_end)
	{
		LOG(ERROR) << "Can not find not space data";
		return false;
	}

	//-- status code

	delim_pos = FindUntilPos(data_begin, data_end, SPACE);
	if (delim_pos == data_end)
	{
		LOG(ERROR) << "Can not find delim of http response status code";
		return false;
	}
	std::string code_str(data_begin, delim_pos);

	try
	{
		status_line.status_code = boost::lexical_cast<uint16>(code_str);
	}
	catch (...)
	{
		LOG(ERROR) << "Invalid status code | " << code_str;
		return false;
	}

	data_begin = FindUntilNotPos(delim_pos, data_end, SPACE);
	if (data_begin == data_end)
	{
		LOG(ERROR) << "Can not find not space data";
		return false;
	}
	
	// skip reason phrase
	
	delim_pos = FindUntilPos2(data_begin, data_end, CR, LF);
	if (delim_pos == data_end)
	{
		LOG(ERROR) << "Can not find end delim of reason phrase";
		return false;
	}

	// skip delim
	data_begin = FindUntilNotPos2(delim_pos, data_end, CR, LF);
	
	parse_len = data_begin - data;

	TMAS_ASSERT(parse_len <= len);

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP请求
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [in] header 请求头
 *         [out] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年02月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpHeader(const char* data, uint32 len, HttpHeader& header, uint32& parse_len)
{
	DLOG(INFO) << "Start to parse http header";

	char* data_begin = const_cast<char*>(data);
	char* data_end = data_begin + len;
	char* delim_pos = data_end;

	while (data_begin < data_end)
	{
		delim_pos = FindUntilPos2(data_begin, data_end, CR, LF);
		if (delim_pos == data_end)
		{
			LOG(ERROR) << "Can not find CR/LF";
			return false;
		}

		char* seprator = std::find(data_begin, delim_pos, ':');
		if (seprator == delim_pos)
		{
			// 找不到":"，认为找到了一个空行，说明消息头结束

			// skip delim
			data_begin = FindUntilNotPos2(delim_pos, data_end, CR, LF);

			parse_len = data_begin - data;
			TMAS_ASSERT(parse_len <= len);

			return true;
		}

		std::string name(data_begin, seprator);
		boost::trim(name);
		boost::to_lower(name);

		if (header.end() == header.find(name))
		{
			std::string value(seprator + 1, delim_pos);
			boost::trim(value);

			header.insert(HttpHeader::value_type(name, value));
		}
		else
		{
			DLOG(WARNING) << "Repeated http header | " << name;
		}

		// skip one CR/LF
		if (*delim_pos == CR && *(delim_pos + 1) == LF)
		{
			data_begin = delim_pos + 2;
		}
		// '\n' is all right
		else if (*delim_pos == LF && *(delim_pos + 1) != CR) 
		{
			data_begin = delim_pos + 1;
		}
		else // '\r' or '\r\n' is invalid
		{
			LOG(ERROR) << "Invalid header delim";
			return false;
		}
	}

	return false; // 没有找到header与body之间的分隔行
}

}

////////////////////////////////////////////////////////////////////////////////

namespace HPN
{

/*-----------------------------------------------------------------------------
 * 描  述: 从HTTP请求中提取出请求头
 *         效率比较低，作为一个暂时方案，后续优化
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] header 请求头
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年05月12日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool GetHttpHeader(const char* data, uint32 len, std::string& header)
{
	std::string tmp(data, len);

	std::string::size_type star_pos = tmp.find_first_of("\r\n");
	if (star_pos == std::string::npos)
	{
		DLOG(WARNING) << "Can not find '\r\n'";
		return false;
	}

	std::string::size_type end_pos = tmp.find("\r\n\r\n");
	if (end_pos == std::string::npos)
	{
		DLOG(WARNING) << "Can not find '\r\n\r\n'";
		return false;
	}

	header = tmp.substr(star_pos, end_pos - star_pos);

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP请求
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] request 请求信息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpRequest(const char* data, uint32 len, HttpRequest& request)
{
	DLOG(INFO) << "Start to parse http request | len: " << len;

	uint32 parse_len = 0;

	char* parse_start = const_cast<char*>(data);
	uint32 data_len = len;

	//--- 解析请求行

	RequestLineParseState rl_state = RLPS_INIT;
	if (!ParseHttpRequestLine(parse_start, len, rl_state, request.request_line, parse_len))
	{
		//LOG(ERROR) << "Fail to parse http request line";
		return false;
	}

	if (rl_state != RLPS_STOP) 
	{
		LOG(ERROR) << "Incomplete http request line";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http request only has request line";
		return false;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	//--- 解析请求头

	HttpHeaderParseState hh_state = HHPS_INIT;
	if (!ParseHttpHeader(parse_start, data_len, hh_state, request.header, parse_len))
	{
		LOG(ERROR) << "Fail to parse http request header";
		return false;
	}

	if (hh_state != HHPS_STOP)
	{
		//LOG(ERROR) << "Incomplete http request header";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http reqeust has no body";
		
		request.body.start = 0;
		request.body.len = 0;
		return true;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	//--- 解析body

	HttpBodyParseState hb_state = HBPS_INIT;
	if (!ParseHttpBody(parse_start, data_len, hb_state, request.body))
	{
		LOG(ERROR) << "Fail to parse http request body";
		return false;
	}

	if (hb_state != HBPS_DATA)
	{
		LOG(ERROR) << "Invalid http body";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP响应
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [out] request 响应信息
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpResponse(const char* data, uint32 len, HttpResponse& response)
{
	uint32 parse_len = 0;

	char* parse_start = const_cast<char*>(data);
	uint32 data_len = len;

	//--- 解析状态行

	StatusLineParseState sl_state = SLPS_INIT;
	if (!ParseHttpStatusLine(parse_start, data_len, sl_state, response.status_line, parse_len))
	{
		LOG(ERROR) << "Fail to parse http status line";
		return false;
	}

	if (sl_state != SLPS_STOP)
	{
		LOG(ERROR) << "Incomplete status line";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http response only has status line";
		return false;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	//--- 解析响应头

	HttpHeaderParseState hh_state = HHPS_INIT;
	if (!ParseHttpHeader(parse_start, data_len, hh_state, response.header, parse_len))
	{
		LOG(ERROR) << "Fail to parse http response header";
		return false;
	}

	if (hh_state != HHPS_STOP)
	{
		LOG(ERROR) << "Incomplete http response header";
		return false;
	}

	if (parse_len == data_len)
	{
		DLOG(WARNING) << "Http response has no body";

		response.body.start = 0;
		response.body.len   = 0;
		return true;
	}

	parse_start += parse_len;
	data_len -= parse_len;

	//--- 解析body

	HttpBodyParseState hb_state = HBPS_INIT;
	if (!ParseHttpBody(parse_start, data_len, hb_state, response.body))
	{
		LOG(ERROR) << "Fail to parse http response body";
		return false;
	}

	if (hb_state != HBPS_DATA)
	{
		LOG(ERROR) << "Invalid http body";
		return false;
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP请求行(基于状态机)
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [in][out] curr_state 当前状态
 *         [out] request_line 请求行信息
 *         [out] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpRequestLine(const char* data, uint32 len, RequestLineParseState& curr_state,
					      HttpRequestLine& request_line, uint32& parse_len)
{
	if (curr_state >= RLPS_STOP) return false;

	char* read_pos = const_cast<char*>(data);
	char* start_pos = read_pos;
	uint32 length = len;

	while (length)
	{
		switch (curr_state)
		{
		case RLPS_INIT:
			if (*read_pos != SPACE)
			{
				start_pos = read_pos;
				curr_state = RLPS_METHOD;
			}
			else
			{
				++read_pos;
				length--;
			}

			break;

		case RLPS_METHOD:
			if (*read_pos != SPACE)
			{
				if (!IsChar(*read_pos)) return false;

				ToUpper(read_pos);

				++read_pos;
				length--;
			}
			else
			{
				request_line.method.assign(start_pos, read_pos);
				curr_state = RLPS_METHOD_POST_SPACE;
			}

			break;

		case RLPS_METHOD_POST_SPACE:
			if (*read_pos != SPACE)
			{
				start_pos = read_pos;
				curr_state = RLPS_URI;
			}
			else
			{
				++read_pos;
				length--;
			}

			break;

		case RLPS_URI:
			if (*read_pos != SPACE)
			{
				++read_pos;
				length--;
			}
			else
			{
				request_line.uri.assign(start_pos, read_pos);
				curr_state = RLPS_URI_POST_SPACE;
			}

			break;

		case RLPS_URI_POST_SPACE:
			if (*read_pos != SPACE)
			{
				start_pos = read_pos;
				curr_state = RLPS_VERSION;	
			}
			else
			{
				++read_pos;
				length--;
			}

			break;

		case RLPS_VERSION:
			if (*read_pos != CR && *read_pos != LF)
			{
				++read_pos;
				length--;
			}
			else
			{
				request_line.version.assign(start_pos, read_pos);
				curr_state = RLPS_VERSION_POST_LF;
			}

			break;

		case RLPS_VERSION_POST_LF:
			if (*read_pos == LF)
			{
				curr_state = RLPS_STOP;
			}

			++read_pos;
			length--;
			
			break;

		case RLPS_STOP:
			parse_len = len - length;
			return true;

		default:
			LOG(ERROR) << "Unexpected state " << curr_state;
			return false;
		}
	}
	
	parse_len = len - length;
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP状态行(基于状态机)
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [in][out] curr_state 当前状态
 *         [out] status_line 状态行信息
 *         [out] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpStatusLine(const char* data, uint32 len, StatusLineParseState& curr_state,
						 HttpStatusLine& status_line, uint32& parse_len)
{
	if (curr_state >= SLPS_STOP) return false;

	char* read_pos = const_cast<char*>(data);
	char* start_pos = read_pos;
	uint32 length = len;

	while (length)
	{
		switch (curr_state)
		{
		case SLPS_INIT:
			if (*read_pos != SPACE)
			{
				start_pos = read_pos;
				curr_state = SLPS_VERSION;
			}
			else
			{
				++read_pos;
				length--;
			}
			break;

		case SLPS_VERSION:
			if (*read_pos != SPACE)
			{
				++read_pos;
				length--;
			}
			else
			{
				status_line.version.assign(start_pos, read_pos);
				curr_state = SLPS_VERSION_POST_SPACE;
			}
			break;

		case SLPS_VERSION_POST_SPACE:
			if (*read_pos != SPACE)
			{
				curr_state = SLPS_STATUS_CODE;
				start_pos = read_pos;
			}
			else
			{
				++read_pos;
				length--;
			}
			break;

		case SLPS_STATUS_CODE:
			if (*read_pos != SPACE)
			{
				if (!IsDigit(*read_pos)) return false;

				++read_pos;
				length--;
			}
			else
			{
				std::string code_str(start_pos, read_pos);
				try
				{
					status_line.status_code = boost::lexical_cast<uint16>(code_str);
				}
				catch (...)
				{
					LOG(ERROR) << "Invalid status code | " << code_str;
					return false;
				}

				curr_state = SLPS_STATUS_CODE_POST_SPACE;
			}
			break;

		case SLPS_STATUS_CODE_POST_SPACE:
			if (*read_pos != SPACE)
			{
				curr_state = SLPS_REASON;
				start_pos = read_pos;
			}
			else
			{
				++read_pos;
				length--;
			}
			break;

		case SLPS_REASON:
			if (*read_pos != CR && *read_pos != LF)
			{
				++read_pos;
				length--;
			}
			else
			{
				curr_state = SLPS_REASON_POST_LF;
			}
			break;

		case SLPS_REASON_POST_LF:
			if (*read_pos == LF)
			{
				curr_state = SLPS_STOP;
			}
			++read_pos;
			length--;

			break;

		case SLPS_STOP:
			parse_len = len - length;
			return true;

		default:
			LOG(ERROR) << "Unexpected state " << curr_state;
			return false;
		}
	}

	parse_len = len - length;
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP头(基于状态机)
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [in][out] curr_state 当前状态
 *         [out] http_header 请求头
 *         [out] parse_len 解析长度
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpHeader(const char* data, uint32 len, HttpHeaderParseState& curr_state,
					 HttpHeader& http_header, uint32& parse_len)
{
	if (curr_state >= HHPS_STOP) return false;

	char* read_pos = const_cast<char*>(data);
	char* start_pos = read_pos;
	uint32 length = len;

	std::string key, value;

	while (length)
	{
		switch (curr_state)
		{
		case HHPS_INIT:
			if (*read_pos == CR || *read_pos == LF)
			{
				curr_state = HHPS_STOP;
			}
			else if (*read_pos != SPACE)
			{
				start_pos = read_pos;
				curr_state = HHPS_KEY;
			}
			else
			{
				++read_pos;
				++start_pos;
				length--;
			}
			break;

		case HHPS_KEY:
			if (*read_pos == COLON)
			{
				// 去除尾部空格
				char* end_pos = read_pos;
				for (; end_pos >= start_pos && *end_pos == SPACE; --end_pos);

				key.assign(start_pos, end_pos);
				curr_state = HHPS_COLON;
			}
			else
			{
				ToLower(read_pos);
			}

			++read_pos;
			length--;

			break;

		case HHPS_COLON:
			if (*read_pos == SPACE)
			{
				++read_pos;
				length--;
			}
			else
			{
				start_pos = read_pos;
				curr_state = HHPS_VALUE;
			}
			break;

		case HHPS_VALUE:
			if (*read_pos != CR && *read_pos != LF)
			{
				++read_pos;
				length--;
			}
			else
			{
				// 去除尾部空格
				char* end_pos = read_pos;
				for (; end_pos >= start_pos && *end_pos == SPACE; --end_pos);

				value.assign(start_pos, end_pos);
				http_header.insert(HttpHeader::value_type(key, value));
				curr_state = HHPS_VALUE_POST_LF;
			}
			break;

		case HHPS_VALUE_POST_LF:
			
			if (*read_pos == LF)
			{
				if (length == 1)
				{
					curr_state = HHPS_STOP;
				}
				else
				{
					curr_state = HHPS_INIT;
				}
			}

			++read_pos;
			length--;

			break;

		case HHPS_STOP:
			parse_len = len - length;
			return true;

		default:
			LOG(ERROR) << "Unexpected state " << curr_state;
			return false;
		}
	}

	parse_len = len - length;
	return true;
}

/*-----------------------------------------------------------------------------
 * 描  述: 解析HTTP消息体(基于状态机)
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 *         [in][out] curr_state 当前状态
 *         [out] http_body 消息体
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool ParseHttpBody(const char* data, uint32 len, HttpBodyParseState& curr_state, HttpBody& http_body)
{
	if (curr_state >= HBPS_DATA) return false;

	http_body.start = 0;
	http_body.len   = 0;

	char* read_pos = const_cast<char*>(data);
	uint32 length = len;

	while (length)
	{
		switch (curr_state)
		{
		case HBPS_INIT:
			if (*read_pos != SPACE)
			{
				curr_state = HBPS_BODY_SEPERATOR;
			}
			else
			{
				++read_pos;
				length--;
			}
			break;

		case HBPS_BODY_SEPERATOR:
			if (*read_pos == LF)
			{	
				curr_state = HBPS_DATA;
			}

			++read_pos;
			length--;

			break;

		case HBPS_DATA:
			http_body.start = read_pos;
			http_body.len   = len - (read_pos - data);
			return true;

		default:
			LOG(ERROR) << "Unexpected state " << curr_state;
			return false;
		}
	}

	return true;
}

} // namespace HPN

/*-----------------------------------------------------------------------------
 * 描  述: 是否是HTTP请求
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月30日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IsHttpRequest(const char* data, uint32 len)
{
	static uint32 get_bit = 0x20544547;

	char* tmp = const_cast<char*>(data);

	return (*(reinterpret_cast<uint32*>(tmp)) == get_bit);
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否是HTTP响应
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月30日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IsHttpResponse(const char* data, uint32 len)
{
	static uint32 response_bit = 0x50545448;

	char* tmp = const_cast<char*>(data);

	return (*(reinterpret_cast<uint32*>(tmp)) == response_bit);
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否包含完整HTTP头
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月30日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool HasCompleteHttpHeader(const char* data, uint32 len)
{
	// TODO: 效率比较低，待优化
	return (std::string(data, len).find("\r\n\r\n") != std::string::npos);
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否是完整的HTTP请求
 * 参  数: [in] data 数据
 *         [in] len 数据长度
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月30日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool IsCompleteHttpRequest(const char* data, uint32 len)
{
	// 大部分HTTP请求不包含数据
	if (*(data + len - 4) == '\r' && *(data + len - 3) == '\n' &&
		*(data + len - 2) == '\r' && *(data + len - 1) == '\n')
	{
		return true;
	}
	// 兼容以'\n'做分隔符的情况
	else if (*(data + len - 2) == '\n' && *(data + len - 1) == '\n')
	{
		return true;
	}
	else
	{
		return false;
	}
}

} // namespace BroadInter