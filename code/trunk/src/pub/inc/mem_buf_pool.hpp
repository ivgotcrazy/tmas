/*#############################################################################
 * �ļ���   : mem_buf_pool.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2013��11��14��
 * �ļ����� : �ڴ������
 * ��Ȩ���� : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_MEM_BUF_POOL
#define BROADINTER_MEM_BUF_POOL

#include <boost/noncopyable.hpp>
#include <boost/pool/pool.hpp>

namespace BroadInter
{

/******************************************************************************
 * ����: �ڴ��
 * ���ߣ�teck_zhou
 * ʱ�䣺2013/11/14
 *****************************************************************************/
class MemBufPool : public boost::noncopyable
{
public:
	static MemBufPool& GetInstance();

	char* AllocPktBuf();
	void FreePktBuf(const char* buf);

	char* AllocPktInfoBuf();
	void FreePktInfoBuf(const char* buf);

private:
	MemBufPool();

private:
	boost::mutex pkt_info_mutex_;
	boost::pool<> pkt_info_buf_pool_;

	boost::mutex pkt_mutex_;
	boost::pool<> pkt_buf_pool_;
};

}

#endif