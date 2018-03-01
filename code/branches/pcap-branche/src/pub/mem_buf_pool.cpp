/*#############################################################################
 * 文件名   : mem_buf_pool.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2013年11月14日
 * 文件描述 : 内存池实现
 * 版权声明 : Copyright (c) 2013 BroadInter. All rights reserved.
 * ##########################################################################*/

#include "mem_buf_pool.hpp"
#include "message.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 获取单例
 * 参  数: 
 * 返回值:
 * 修  改:
 * 时间 2013年11月14日
 * 作者 teck_zhou
 * 描述 创建
 ----------------------------------------------------------------------------*/
MemBufPool& MemBufPool::GetInstance()
{
	static MemBufPool pool;
	return pool;
}

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
MemBufPool::MemBufPool() : pkt_info_buf_pool_(sizeof(PktInfo), 1024 * 8)
	, pkt_buf_pool_(1600, 1024 * 8)
{
}

/*-----------------------------------------------------------------------------
 * 描  述: 申请报文缓存
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
char* MemBufPool::AllocPktBuf()
{
	boost::mutex::scoped_lock lock(pkt_mutex_);

	return static_cast<char*>(pkt_buf_pool_.ordered_malloc());
}

/*-----------------------------------------------------------------------------
 * 描  述: 释放报文缓存
 * 参  数: [in] buf 缓存
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void MemBufPool::FreePktBuf(const char* buf)
{
	boost::mutex::scoped_lock lock(pkt_mutex_);

	pkt_buf_pool_.ordered_free(const_cast<char*>(buf));
}

/*-----------------------------------------------------------------------------
 * 描  述: 申请报文缓存
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
char* MemBufPool::AllocPktInfoBuf()
{
	boost::mutex::scoped_lock lock(pkt_info_mutex_);

	return static_cast<char*>(pkt_info_buf_pool_.ordered_malloc());
}

/*-----------------------------------------------------------------------------
 * 描  述: 释放报文缓存
 * 参  数: [in] buf 缓存
 * 返回值: 
 * 修  改:
 *   时间 2013年11月14日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void MemBufPool::FreePktInfoBuf(const char* buf)
{
	boost::mutex::scoped_lock lock(pkt_info_mutex_);

	pkt_info_buf_pool_.ordered_free(const_cast<char*>(buf));
}

}

