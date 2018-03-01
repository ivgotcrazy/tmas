/*#############################################################################
 * �ļ���   : timer.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��10��
 * �ļ����� : Timer����
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_TIMER
#define BROADINTER_TIMER

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include "tmas_typedef.hpp"

namespace BroadInter
{

/******************************************************************************
 * ����: ��boost��ʱ���ķ�װ�����߳��첽��ʱ��
 * ���ߣ�teck_zhou
 * ʱ�䣺2014/02/26
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