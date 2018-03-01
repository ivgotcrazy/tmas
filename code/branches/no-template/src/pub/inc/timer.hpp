/*#############################################################################
 * 文件名   : timer.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年02月10日
 * 文件描述 : Timer声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TIMER
#define BROADINTER_TIMER

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

/******************************************************************************
 * 描述: 对boost定时器的封装，多线程异步定时器
 * 作者：teck_zhou
 * 时间：2014/02/26
 *****************************************************************************/
class FreeTimer : public boost::noncopyable
{
public:
	typedef boost::function<void()> TimerCallback;

	FreeTimer(TimerCallback callback, uint32 interval);
	~FreeTimer();

	void Start();
	void Stop();

private:
	void OnTick(const error_code& ec);

	typedef boost::scoped_ptr<boost::thread> ScopedThread;

private:
	io_service		ios_;
	deadline_timer	timer_;
	TimerCallback	callback_;
	uint32			interval_;
	ScopedThread	timer_thread_;
	bool			stop_flag_;
};

}

#endif