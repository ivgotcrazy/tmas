/*#############################################################################
 * �ļ���   : http_recombinder_impl.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��01��
 * �ļ����� : ���������ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <glog/logging.h>

#include "http_recombinder_impl.hpp"
#include "tmas_assert.hpp"
#include "http_run_session.hpp"

namespace BroadInter
{

static bool IsHexDigit(char digit)
{
	return ( (digit >= 0x30 && digit <= 0x39) /* 0-9 */
		|| (digit >= 0x41 && digit <= 0x46) /* A-F */
		|| (digit >= 0x61 && digit <= 0x66) /* a-f */ ) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] run_session ����RunSession
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
UnknownRecombinder::UnknownRecombinder(HttpRunSession* run_session)
	: run_session_(run_session)
{

}

/*------------------------------------------------------------------------------
 * ��  ��: ���Ľ���
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void UnknownRecombinder::AppendData(const char* data, uint32 len)
{
	return run_session_->RecombindCallback(true, data, len);
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] handler ���鱨�Ĵ�����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
ContentLengthRecombinder::ContentLengthRecombinder(HttpRunSession* run_session
	, bool copy_data)
	: appended_size_(0)
	, content_length_(0)
	, run_session_(run_session)
	, copy_data_(copy_data)
{
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: [in] content_length ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void ContentLengthRecombinder::Init(uint32 content_length)
{
	pkt_buf_.clear();

	content_length_ = content_length;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����"content-length"��HTTP���������
 * ��  ��: [in] data ��������
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void ContentLengthRecombinder::AppendData(const char* data, uint32 len)
{
	// ���Content-LengthΪ0��ֻҪ����Ӧ����Ϊ�Ѿ���ɻỰ
	if (content_length_ == 0)
	{
		return run_session_->RecombindCallback(true, 0, 0);
	}

	// string��size�������ܽϲ����ʹ�ø�����Ա
	appended_size_ += len;

	// ��Ӧ�����������Ȼ�������
	if (appended_size_ < content_length_)
	{
		if (copy_data_) pkt_buf_.append(data, len);
		return;
	}
	else if (appended_size_ > content_length_)
	{
		LOG(WARNING) << "Appended data is bigger than content-length | "
					 << "content-length: " << content_length_ 
					 << " appended: " << appended_size_;
	}

	// �ߵ���˵�������Ѿ�������

	if (copy_data_)
	{
		pkt_buf_.append(data, len);
		run_session_->RecombindCallback(true, pkt_buf_.data(), content_length_);

		pkt_buf_.clear();
	}
	else
	{
		run_session_->RecombindCallback(true, 0, content_length_);
	}
}

////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] handler ������
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
ChunkedRecombinder::ChunkedRecombinder(HttpRunSession* run_session, bool copy_data)
	: parse_data_len_(0), run_session_(run_session), copy_data_(copy_data)
{
	Init();
}

/*------------------------------------------------------------------------------
 * ��  ��: ��ʼ��
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void ChunkedRecombinder::Init()
{
	pkt_buf_.clear();

	curr_state_ = CHUNK_HEX;
	has_trailer_ = false;
	chunk_size_ = 0;
	hex_index_ = 0;
}

/*------------------------------------------------------------------------------
 * ��  ��: ����"transfer-encoding: chunked"��HTTP���������
 * ��  ��: [in] data ��������
 *         [in] len ���ݳ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��04��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
void ChunkedRecombinder::AppendData(const char* data, uint32 len)
{
	uint32 parse_len = 0;
	if (!ProcChunkFsm(data, len, pkt_buf_, parse_len))
	{
		LOG(ERROR) << "Fail to process chunk fsm";
		run_session_->RecombindCallback(false, 0, 0);
		return;
	}

	TMAS_ASSERT(parse_len <= len);

	if (curr_state_ != CHUNK_STOP) return;

	if (copy_data_)
	{
		run_session_->RecombindCallback(true, pkt_buf_.c_str(), pkt_buf_.length());
		pkt_buf_.clear();
	}
	else
	{
		run_session_->RecombindCallback(true, 0, parse_data_len_);
		parse_data_len_ = 0;
	}
}

/*------------------------------------------------------------------------------
 * ��  ��: ���Ľ���
 * ��  ��: [in] data ����
 *         [in] len ���ݳ���
 *         [out] pkt_buf ���ݻ���
 *         [out] parse_len ��������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��03��
 *   ���� teck_zhou
 *   ���� ����
 -----------------------------------------------------------------------------*/
bool ChunkedRecombinder::ProcChunkFsm(const char* data, uint32 len, 
	                                  std::string& pkt_buf, uint32& parse_len)
{
	char* datap = const_cast<char*>(data);
	uint32 length = len;
	uint32 piece = 0;	

	while(length) 
	{
		switch(curr_state_) 
		{
		case CHUNK_HEX:
			if(IsHexDigit(*datap))
			{
				if(hex_index_ < kMaxHexSize) 
				{
					hex_buf_[hex_index_] = *datap;
					datap++;
					length--;
					hex_index_++;
				}
				else 
				{
					LOG(ERROR) << "longer hex than we support";
					return false; /* longer hex than we support */
				}
			}
			else
			{
				char *endptr;
				if(0 == hex_index_)
				{
					LOG(ERROR) << "This is illegal data, we received junk where we expected a hexadecimal digit.";
					return false;
				}

				/* length and datap are unmodified */
				hex_buf_[hex_index_]=0;

				chunk_size_ = strtoul(hex_buf_, &endptr, 16);
				if(errno == ERANGE)
				{
					LOG(ERROR) << "over or underflow is an error";
					return false;
				}

				curr_state_ = CHUNK_LF; /* now wait for the CRLF */
			}
			break;

		case CHUNK_LF:
			/* waiting for the LF after a chunk size */
			if(*datap == 0x0a) 
			{
				/* we're now expecting data to come, unless size was zero! */
				if(0 == chunk_size_) 
				{
					curr_state_ = CHUNK_TRAILER; /* now check for trailers */
				}
				else
				{
					curr_state_ = CHUNK_DATA;
				}
			}
			datap++;
			length--;
			break;

		case CHUNK_DATA:
			/* We expect 'datasize' of data. We have 'length' right now, it can be
			   more or less than 'datasize'. Get the smallest piece.
			*/
			piece = (chunk_size_ >= length) ? length : chunk_size_;

			if (copy_data_)
			{
				pkt_buf.append(datap, piece);
			}

			parse_data_len_ += piece;
			
			chunk_size_ -= piece; /* decrease amount left to expect */
			datap += piece;    /* move read pointer forward */
			length -= piece;   /* decrease space left in this round */

			if(0 == chunk_size_)
			{
				/* end of data this round, we now expect a trailing CRLF */
				curr_state_ = CHUNK_POSTLF;
			}
			break;

		case CHUNK_POSTLF:
			if(*datap == 0x0a) 
			{
				/* The last one before we go back to hex state and start all over. */
				/* sets state back to CHUNK_HEX */

				chunk_size_ = 0;
				curr_state_ = CHUNK_HEX;
				has_trailer_ = false;
				hex_index_ = 0;
			}
			else if(*datap != 0x0d)
			{
				LOG(ERROR) << "CHUNKE_BAD_CHUNK";
				return false;
			}
			
			datap++;
			length--;

			break;

		case CHUNK_TRAILER:
			if((*datap == 0x0d) || (*datap == 0x0a)) 
			{
				if (has_trailer_)
				{
					curr_state_ = CHUNK_TRAILER_CR;
					if (*datap == 0x0a) break;
				}
				else
				{
					curr_state_ = CHUNK_TRAILER_POSTCR;
					break;
				}
			}
			else
			{
				if (!has_trailer_) has_trailer_ = true;
			}

			datap++;
			length--;

			break;

		case CHUNK_TRAILER_CR:
			if(*datap == 0x0a) 
			{
				curr_state_ = CHUNK_TRAILER_POSTCR;
				datap++;
				length--;
			}
			else
			{
				LOG(ERROR) << "CHUNKE_BAD_CHUNK";
				return false;
			}
			break;

		case CHUNK_TRAILER_POSTCR:
			/* We enter this state when a CR should arrive so we expect to
			   have to first pass a CR before we wait for LF */
			if((*datap != 0x0d) && (*datap != 0x0a)) 
			{
				/* not a CR then it must be another header in the trailer */
				curr_state_ = CHUNK_TRAILER;
				break;
			}
			if(*datap == 0x0d) 
			{
				/* skip if CR */
				datap++;
				length--;
			}
			/* now wait for the final LF */
			curr_state_ = CHUNK_STOP;
			break;

		case CHUNK_STOP:
			if(*datap == 0x0a) 
			{
				length--;

				/* Record the length of any data left in the end of the buffer
				   even if there's no more chunks to read */

				parse_len = datap - data;

				return true; /* return stop */
			}
			else
			{
				LOG(ERROR) << "CHUNKE_BAD_CHUNK";
				return false;
			}
			break;

		default:
			TMAS_ASSERT(false);
			return false;
		}

	}

	return true;
}

}