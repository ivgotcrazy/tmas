/*#############################################################################
 * �ļ���   : timer.cpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��02��10��
 * �ļ����� : Timerʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "timer.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] callback �ص�����
 *         [in] interval ��ʱ��ʱ��
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��02��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
FreeTimer::FreeTimer(TimerCallback callback, uint32 interval)
	: ios_(), timer_(ios_), callback_(callback)
	, interval_(interval), stop_flag_(false)
{
	TMAS_ASSERT(callback_);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��02��26��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ��ʱ���ص�����
 * ��  ��: [in] ec ������
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void FreeTimer::OnTick(const error_code& ec)
{
	if (stop_flag_ || ec) return;

	if (callback_) callback_();

	timer_.expires_from_now(boost::posix_time::seconds(interval_));
	timer_.async_wait(boost::bind(&FreeTimer::OnTick, this, _1));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������ʱ��
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2013��11��10��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void FreeTimer::Start()
{
	timer_thread_.reset(new boost::thread(boost::bind(&io_service::run, &ios_)));

	timer_.expires_from_now(boost::posix_time::seconds(interval_));
	timer_.async_wait(boost::bind(&FreeTimer::OnTick, this, _1));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ֹͣ��ʱ��
 * ��  ��: 
 * ����ֵ:
 * ��  ��:
 *   ʱ�� 2014��03��16��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
void FreeTimer::Stop()
{
	stop_flag_ = true;
}

}