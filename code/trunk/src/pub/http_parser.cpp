/*#############################################################################
 * �ļ���   : http_parser.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��12��
 * �ļ����� : HttpParser��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
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
 * ��  ��: ����HTTP����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] request ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����HTTP��Ӧ
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] request ��Ӧ��Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����HTTP������
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] request_line ��������Ϣ
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
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
	
	// �ӵ�ǰ�ո񴦿�ʼѰ�ҵ�һ���ǿո�λ��
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

	// �ӵ�ǰ�ո񴦿�ʼѰ�ҵ�һ���ǿո�λ��
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
 * ��  ��: ����HTTP��Ӧ��
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] response ��Ӧ��Ϣ
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����HTTP����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [in] header ����ͷ
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��02��12��
 *   ���� teck_zhou
 *   ���� ����
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
			// �Ҳ���":"����Ϊ�ҵ���һ�����У�˵����Ϣͷ����

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

	return false; // û���ҵ�header��body֮��ķָ���
}

}

////////////////////////////////////////////////////////////////////////////////

namespace HPN
{

/*-----------------------------------------------------------------------------
 * ��  ��: ��HTTP��������ȡ������ͷ
 *         Ч�ʱȽϵͣ���Ϊһ����ʱ�����������Ż�
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] header ����ͷ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��05��12��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����HTTP����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] request ������Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool ParseHttpRequest(const char* data, uint32 len, HttpRequest& request)
{
	DLOG(INFO) << "Start to parse http request | len: " << len;

	uint32 parse_len = 0;

	char* parse_start = const_cast<char*>(data);
	uint32 data_len = len;

	//--- ����������

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

	//--- ��������ͷ

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

	//--- ����body

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
 * ��  ��: ����HTTP��Ӧ
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] request ��Ӧ��Ϣ
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��14��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool ParseHttpResponse(const char* data, uint32 len, HttpResponse& response)
{
	uint32 parse_len = 0;

	char* parse_start = const_cast<char*>(data);
	uint32 data_len = len;

	//--- ����״̬��

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

	//--- ������Ӧͷ

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

	//--- ����body

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
 * ��  ��: ����HTTP������(����״̬��)
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [in][out] curr_state ��ǰ״̬
 *         [out] request_line ��������Ϣ
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��14��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����HTTP״̬��(����״̬��)
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [in][out] curr_state ��ǰ״̬
 *         [out] status_line ״̬����Ϣ
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��14��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ����HTTPͷ(����״̬��)
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [in][out] curr_state ��ǰ״̬
 *         [out] http_header ����ͷ
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��14��
 *   ���� teck_zhou
 *   ���� ����
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
				// ȥ��β���ո�
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
				// ȥ��β���ո�
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
 * ��  ��: ����HTTP��Ϣ��(����״̬��)
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [in][out] curr_state ��ǰ״̬
 *         [out] http_body ��Ϣ��
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��14��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �Ƿ���HTTP����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��30��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool IsHttpRequest(const char* data, uint32 len)
{
	static uint32 get_bit = 0x20544547;

	char* tmp = const_cast<char*>(data);

	return (*(reinterpret_cast<uint32*>(tmp)) == get_bit);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ���HTTP��Ӧ
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��30��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool IsHttpResponse(const char* data, uint32 len)
{
	static uint32 response_bit = 0x50545448;

	char* tmp = const_cast<char*>(data);

	return (*(reinterpret_cast<uint32*>(tmp)) == response_bit);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ��������HTTPͷ
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��30��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool HasCompleteHttpHeader(const char* data, uint32 len)
{
	// TODO: Ч�ʱȽϵͣ����Ż�
	return (std::string(data, len).find("\r\n\r\n") != std::string::npos);
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ���������HTTP����
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��30��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool IsCompleteHttpRequest(const char* data, uint32 len)
{
	// �󲿷�HTTP���󲻰�������
	if (*(data + len - 4) == '\r' && *(data + len - 3) == '\n' &&
		*(data + len - 2) == '\r' && *(data + len - 1) == '\n')
	{
		return true;
	}
	// ������'\n'���ָ��������
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