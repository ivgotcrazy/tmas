/*#############################################################################
 * �ļ���   : http_recombinder_impl.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��01��
 * �ļ����� : �������������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RECOMBINDER_IMPL
#define BROADINTER_HTTP_RECOMBINDER_IMPL

#include <boost/function.hpp>

#include <string>

#include "tmas_typedef.hpp"
#include "http_typedef.hpp"

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: HTTP��������ʵ�ֽӿ�
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��03��
 ******************************************************************************/
class HttpRecombinderImpl
{
public:
	virtual void AppendData(const char* data, uint32 len) = 0;
};

/*******************************************************************************
 * ��  ��: δָ�����ݴ������͵�HTTP����������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��04��03��
 ******************************************************************************/
class UnknownRecombinder : public HttpRecombinderImpl
{
public:
	UnknownRecombinder(HttpRunSession* run_session);

	virtual void AppendData(const char* data, uint32 len) override;

private:
	HttpRunSession* run_session_;
};

/*******************************************************************************
 * ��  ��: ����"content-length"��HTTP����������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��03��
 ******************************************************************************/
class ContentLengthRecombinder : public HttpRecombinderImpl
{
public:
	ContentLengthRecombinder(HttpRunSession* run_session, bool copy_data);

	void Init(uint32 content_length);

	virtual void AppendData(const char* data, uint32 len) override;

	void SetContentLength(uint32 len);

private:
	std::string pkt_buf_;

	uint32 appended_size_;

	uint32 content_length_;

	HttpRunSession* run_session_;

	bool copy_data_;
};

/*******************************************************************************
 * ��  ��: ����"transfer-encoding: chunked"��HTTP����������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��03��
 ******************************************************************************/
class ChunkedRecombinder : public HttpRecombinderImpl
{
public:
	ChunkedRecombinder(HttpRunSession* run_session, bool copy_data);

	void Init();

	virtual void AppendData(const char* data, uint32 len) override;

private:
	enum ChunkState
	{
		CHUNK_HEX,
		CHUNK_LF,
		CHUNK_DATA,
		CHUNK_POSTLF,
		CHUNK_STOP,
		CHUNK_TRAILER,
		CHUNK_TRAILER_CR,
		CHUNK_TRAILER_POSTCR,
		CHUNK_BUTT
	};

private:
	bool ProcChunkFsm(const char* data, uint32 len, std::string& pkt_buf, uint32& parse_len);

private:
	std::string pkt_buf_;

	uint32 parse_data_len_;

	ChunkState curr_state_;
	
	bool has_trailer_;

	uint32 chunk_size_;

	static const uint8 kMaxHexSize = 8;

	char hex_buf_[kMaxHexSize + 1];
	uint8 hex_index_;

	HttpRunSession* run_session_;

	bool copy_data_;
};

#if 0 // ��ɾ������ע��
enum ChunkState
{
	/* await and buffer all hexadecimal digits until we get one that isn't a
		hexadecimal digit. When done, we go CHUNK_LF */
	CHUNK_HEX,

	/* wait for LF, ignore all else */
	CHUNK_LF,

	/* We eat the amount of data specified. When done, we move on to the
		POST_CR state. */
	CHUNK_DATA,

	/* POSTLF should get a CR and then a LF and nothing else, then move back to
		HEX as the CRLF combination marks the end of a chunk. A missing CR is no
		big deal. */
	CHUNK_POSTLF,

	/* Used to mark that we're out of the game.  NOTE: that there's a 'dataleft'
		field in the struct that will tell how many bytes that were not passed to
		the client in the end of the last buffer! */
	CHUNK_STOP,

	/* At this point optional trailer headers can be found, unless the next line
		is CRLF */
	CHUNK_TRAILER,

	/* A trailer CR has been found - next state is CHUNK_TRAILER_POSTCR.
		Next char must be a LF */
	CHUNK_TRAILER_CR,

	/* A trailer LF must be found now, otherwise CHUNKE_BAD_CHUNK will be
		signalled If this is an empty trailer CHUNKE_STOP will be signalled.
		Otherwise the trailer will be broadcasted via Curl_client_write() and the
		next state will be CHUNK_TRAILER */
	CHUNK_TRAILER_POSTCR
};
#endif

}

#endif