/*#############################################################################
 * 文件名   : timer.cpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月10日
 * 文件描述 : Timer实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "timer.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] callback 回调函数
 *         [in] interval 定时器时长
 * 返回值:
 * 修  改:
 *   时间 2014年02月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
FreeTimer::FreeTimer(TimerCallback callback, uint32 interval)
	: ios_(), timer_(ios_), callback_(callback)
	, interval_(interval), stop_flag_(false)
{
	TMAS_ASSERT(callback_);
}

/*-----------------------------------------------------------------------------
 * 描  述: 析构函数
 * 返回值:
 * 修  改:
 *   时间 2014年02月26日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
FreeTimer::~FreeTimer()
{
	timer_thread_->interrupt();
	if (timer_thread_->joinable())
	{
		timer_thread_->timed_join(boost::posix_time::millisec(10));
	}

	ios_.stop();
}

/*-----------------------------------------------------------------------------
 * 描  述: 定时器回调函数
 * 参  数: [in] ec 错误码
 * 返回值:
 * 修  改:
 *   时间 2013年11月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void FreeTimer::OnTick(const error_code& ec)
{
	if (stop_flag_ || ec) return;

	if (callback_) callback_();

	timer_.expires_from_now(boost::posix_time::seconds(interval_));
	timer_.async_wait(boost::bind(&FreeTimer::OnTick, this, _1));
}

/*-----------------------------------------------------------------------------
 * 描  述: 启动定时器
 * 参  数: 
 * 返回值:
 * 修  改:
 *   时间 2013年11月10日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void FreeTimer::Start()
{
	timer_thread_.reset(new boost::thread(boost::bind(&io_service::run, &ios_)));

	timer_.expires_from_now(boost::posix_time::seconds(interval_));
	timer_.async_wait(boost::bind(&FreeTimer::OnTick, this, _1));
}

/*-----------------------------------------------------------------------------
 * 描  述: 停止定时器
 * 参  数: 
 * 返回值:
 * 修  改:
 *   时间 2014年03月16日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
void FreeTimer::Stop()
{
	stop_flag_ = true;
}

}