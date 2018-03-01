/*#############################################################################
 * �ļ���   : chunked_http_recombinder_test.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��04��
 * �ļ����� : HTTP�����������ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <boost/bind.hpp>

#include "chunked_http_recombinder_test.hpp"

namespace BroadInter
{

//==============================================================================
// Test class implementation
//==============================================================================

ChunkedHttpRecombinderTest::ChunkedHttpRecombinderTest()
	: recombinder_(boost::bind(&ChunkedHttpRecombinderTest::ReceviePacket, this, _1, _2, _3))
{

}

ChunkedHttpRecombinderTest::~ChunkedHttpRecombinderTest()
{

}

void ChunkedHttpRecombinderTest::SetUp()
{
	call_result_ = false;
	called_times_ = 0;
	pkt_buf_.clear();
}

void ChunkedHttpRecombinderTest::TearDown()
{

}

void ChunkedHttpRecombinderTest::ReceviePacket(bool result, const char* data, uint32 len)
{
	called_times_++;

	call_result_ = result;

	if (result)
	{
		pkt_buf_.append(data, len);
	}	
}

//==============================================================================
// Test case implementation
//==============================================================================

/*------------------------------------------------------------------------------
 * ����: һ�������а�������chunk
 *       1) ��һ��chunkΪ10�ֽ�
 *       2) �ڶ���chunkΪ29�ֽ�
 *       3) ������chunkΪ8�ֽ�
 *----------------------------------------------------------------------------*/
TEST_F(ChunkedHttpRecombinderTest, three_chunks_in_one_pkt)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char response[] = 
	{
		0x61, 0x0d, 0x0a, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0d, 0x0a, 0x31,
		0x64, 0x0d, 0x0a, 0xab, 0x56, 0x4a, 0x2d, 0x2a, 0xf2, 0xcb, 0x57, 0xb2, 0x52, 0x32, 0x50, 0xd2,
		0x51, 0xca, 0x2c, 0x76, 0xca, 0x2c, 0x2a, 0xc9, 0x48, 0x49, 0xac, 0x04, 0x0b, 0xd4, 0x02, 0x00,
		0x0d, 0x0a, 0x38, 0x0d, 0x0a, 0x63, 0x9e, 0x1b, 0xd2, 0x1e, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30,
		0x0d, 0x0a, 0x0d, 0x0a
	};
	
#pragma GCC diagnostic warning "-Wnarrowing"

	recombinder_.Init();

	recombinder_.AppendData(response, sizeof(response));

	ASSERT_EQ(1, called_times_);
	ASSERT_TRUE(call_result_);
	ASSERT_EQ(47, pkt_buf_.length());
	ASSERT_EQ(0x1f, (uint8)pkt_buf_.c_str()[0]);
}

/*------------------------------------------------------------------------------
 * ����: ���������а�������chunk
 *       1) ��һ�����İ�����һ������chunk�Լ��ڶ���chunk�ĳ���
 *       2) �ڶ������İ����ڶ���chunkʣ����ֽ��Լ�������chunk�ĳ��ȺͲ���data
 *       3) ���������İ���������chunkΪʣ���data
 *----------------------------------------------------------------------------*/
TEST_F(ChunkedHttpRecombinderTest, three_chunks_in_three_pkts)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char response1[] = 
	{
		0x61, 0x0d, 0x0a, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0d, 0x0a, 0x31,
		0x64
	};

	char response2[] = 
	{
			  0x0d, 0x0a, 0xab, 0x56, 0x4a, 0x2d, 0x2a, 0xf2, 0xcb, 0x57, 0xb2, 0x52, 0x32, 0x50, 0xd2,
		0x51, 0xca, 0x2c, 0x76, 0xca, 0x2c, 0x2a, 0xc9, 0x48, 0x49, 0xac, 0x04, 0x0b, 0xd4, 0x02, 0x00,
		0x0d, 0x0a, 0x38, 0x0d, 0x0a, 0x63, 0x9e, 0x1b, 
	};

	char response3[] = 
	{
														0xd2, 0x1e, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30,
		0x0d, 0x0a, 0x0d, 0x0a
	};

#pragma GCC diagnostic warning "-Wnarrowing"
	
	recombinder_.Init();

	recombinder_.AppendData(response1, sizeof(response1));
	ASSERT_EQ(0, called_times_);

	recombinder_.AppendData(response2, sizeof(response2));
	ASSERT_EQ(0, called_times_);

	recombinder_.AppendData(response3, sizeof(response3));
	ASSERT_EQ(1, called_times_);
	ASSERT_TRUE(call_result_);
	ASSERT_EQ(47, pkt_buf_.length());
	ASSERT_EQ(0x1f, (uint8)pkt_buf_.c_str()[0]);
}

/*------------------------------------------------------------------------------
 * ����: chunk���ȳ����������(8��16�����ַ���ʾ��4�ֽ�����)
 *       ��һ��chunk����Ϊ9��a��ʾ�ĳ���(A AA AA AA AA)
 *----------------------------------------------------------------------------*/
TEST_F(ChunkedHttpRecombinderTest, too_large_chunk_length)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char response[] = 
	{
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x0d, 0x0a, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0d, 0x0a, 0x31,
		0x64, 0x0d, 0x0a, 0xab, 0x56, 0x4a, 0x2d, 0x2a, 0xf2, 0xcb, 0x57, 0xb2, 0x52, 0x32, 0x50, 0xd2,
		0x51, 0xca, 0x2c, 0x76, 0xca, 0x2c, 0x2a, 0xc9, 0x48, 0x49, 0xac, 0x04, 0x0b, 0xd4, 0x02, 0x00,
		0x0d, 0x0a, 0x38, 0x0d, 0x0a, 0x63, 0x9e, 0x1b, 0xd2, 0x1e, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30,
		0x0d, 0x0a, 0x0d, 0x0a
	};

#pragma GCC diagnostic warning "-Wnarrowing"
	
	recombinder_.Init();

	recombinder_.AppendData(response, sizeof(response));

	ASSERT_EQ(1, called_times_);
	ASSERT_FALSE(call_result_);
}

/*------------------------------------------------------------------------------
 * ����: �����chunk���ȣ���һ��chunk����Ϊ:AAAAAAAA��ʵ�ʳ���Ϊ10�ֽ�
 *----------------------------------------------------------------------------*/
TEST_F(ChunkedHttpRecombinderTest, wrong_chunk_length)
{
#pragma GCC diagnostic ignored "-Wnarrowing"

	char response[] = 
	{
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x0d, 0x0a, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0d, 0x0a, 0x31,
		0x64, 0x0d, 0x0a, 0xab, 0x56, 0x4a, 0x2d, 0x2a, 0xf2, 0xcb, 0x57, 0xb2, 0x52, 0x32, 0x50, 0xd2,
		0x51, 0xca, 0x2c, 0x76, 0xca, 0x2c, 0x2a, 0xc9, 0x48, 0x49, 0xac, 0x04, 0x0b, 0xd4, 0x02, 0x00,
		0x0d, 0x0a, 0x38, 0x0d, 0x0a, 0x63, 0x9e, 0x1b, 0xd2, 0x1e, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30,
		0x0d, 0x0a, 0x0d, 0x0a
	};

#pragma GCC diagnostic warning "-Wnarrowing"
	
	recombinder_.Init();

	recombinder_.AppendData(response, sizeof(response));

	ASSERT_EQ(0, called_times_);
}

/*------------------------------------------------------------------------------
 * ����: chunk���ȳ����������(8��16�����ַ���ʶ��4�ֽ�����)
 *       ��һ��chunk����Ϊ9��a��ʾ�ĳ���(A AA AA AA AA)
 *----------------------------------------------------------------------------*/
TEST_F(ChunkedHttpRecombinderTest, invalid_chunk_length_data)
{

#pragma GCC diagnostic ignored "-Wnarrowing"

	char response[] = 
	{
		0xff, 0x0d, 0x0a, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0d, 0x0a, 0x31,
		0x64, 0x0d, 0x0a, 0xab, 0x56, 0x4a, 0x2d, 0x2a, 0xf2, 0xcb, 0x57, 0xb2, 0x52, 0x32, 0x50, 0xd2,
		0x51, 0xca, 0x2c, 0x76, 0xca, 0x2c, 0x2a, 0xc9, 0x48, 0x49, 0xac, 0x04, 0x0b, 0xd4, 0x02, 0x00,
		0x0d, 0x0a, 0x38, 0x0d, 0x0a, 0x63, 0x9e, 0x1b, 0xd2, 0x1e, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x30,
		0x0d, 0x0a, 0x0d, 0x0a
	};
	
#pragma GCC diagnostic warning "-Wnarrowing"

	recombinder_.Init();

	recombinder_.AppendData(response, sizeof(response));

	ASSERT_EQ(1, called_times_);
	ASSERT_FALSE(call_result_);
}

}