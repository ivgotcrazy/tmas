/*#############################################################################
 * 文件名   : http_recombinder_impl.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月01日
 * 文件描述 : 重组策略类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_HTTP_RECOMBINDER_IMPL
#define BROADINTER_HTTP_RECOMBINDER_IMPL

#include <boost/function.hpp>

#include <string>

#include "tmas_typedef.hpp"
#include "http_typedef.hpp"

namespace BroadInter
{

typedef boost::function<void(bool, const char*, uint32)> RecombindHandler;

/*******************************************************************************
 * 描  述: HTTP报文重组实现接口
 * 作  者: teck_zhou
 * 时  间: 2014年03月03日
 ******************************************************************************/
class HttpRecombinderImpl
{
public:
	virtual void AppendData(const char* data, uint32 len) = 0;
};

/*******************************************************************************
 * 描  述: 基于"content-length"的HTTP报文重组器
 * 作  者: teck_zhou
 * 时  间: 2014年03月03日
 ******************************************************************************/
class ContentLengthRecombinder : public HttpRecombinderImpl
{
public:
	ContentLengthRecombinder(const RecombindHandler& handler);

	void Init(uint32 content_length);

	virtual void AppendData(const char* data, uint32 len) override;

	void SetContentLength(uint32 len);

private:
	std::string pkt_buf_;

	uint32 content_length_;

	RecombindHandler handler_;
};

/*******************************************************************************
 * 描  述: 基于"transfer-encoding: chunked"的HTTP报文重组器
 * 作  者: teck_zhou
 * 时  间: 2014年03月03日
 ******************************************************************************/
class ChunkedRecombinder : public HttpRecombinderImpl
{
public:
	ChunkedRecombinder(const RecombindHandler& handler);

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
	bool ProcChunkFsm(const char* data, uint32 len, 
		              std::string& pkt_buf, uint32& parse_len);

private:
	std::string pkt_buf_;

	ChunkState curr_state_;
	
	bool has_trailer_;

	uint32 chunk_size_;

	static const uint8 kMaxHexSize = 8;

	char hex_buf_[kMaxHexSize + 1];
	uint8 hex_index_;

	RecombindHandler handler_;
};

#if 0 // 勿删，保留注释
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