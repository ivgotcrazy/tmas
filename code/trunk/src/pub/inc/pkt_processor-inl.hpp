/*#############################################################################
 * �ļ���   : pkt_processor-inl.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��23��
 * �ļ����� : PktProcessor��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_PKT_PROCESSOR_INL
#define BROADINTER_PKT_PROCESSOR_INL

#include "pkt_processor.hpp"

namespace BroadInter
{
/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ������ָ��
 * ��  ��: 
 * ����ֵ: ������ָ��
 * ��  ��: 
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline Drived* PktProcessor<Drived, Next, Succ>::GetDrived()
{
	return static_cast<Drived*>(this);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����Ǳ�����Ϣ
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ������
 * ��  ��: 
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline ProcInfo PktProcessor<Drived, Next, Succ>::ProcessMsg(MsgType msg_type, void* msg_data)
{
	// �ȵ��ñ������������������Ƿ�����
	GetDrived()->DoProcessMsg(msg_type, msg_data);

	if (next_processor_) // ���������´���
	{
		next_processor_->DoProcessMsg(msg_type, msg_data);
	}

	return PI_HAS_PROCESSED;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ü��δ������ķǱ��Ĵ���
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline void PktProcessor<Drived, Next, Succ>::PassMsgToSuccProcessor(MsgType msg_type, void* msg_data)
{
	if (succ_processor_)
	{
		succ_processor_->ProcessMsg(msg_type, msg_data);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������һ������
 * ��  ��: [in] processor ������
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline void PktProcessor<Drived, Next, Succ>::SetNextProcessor(boost::shared_ptr<Next> processor)
{
	next_processor_ = processor;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���ü��δ�����
 * ��  ��: [in] processor ������
 * ����ֵ: 
 * ��  ��: 
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline void PktProcessor<Drived, Next, Succ>::SetSuccProcessor(boost::shared_ptr<Succ> processor)
{
	succ_processor_ = processor;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ��һ������
 * ��  ��: 
 * ����ֵ: ������
 * ��  ��: 
 *   ʱ�� 2014��03��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline boost::shared_ptr<Next> PktProcessor<Drived, Next, Succ>::GetNextProcessor() const
{
	return next_processor_;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ���δ�����
 * ��  ��: 
 * ����ֵ: ������
 * ��  ��: 
 *   ʱ�� 2014��03��24��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline boost::shared_ptr<Succ> PktProcessor<Drived, Next, Succ>::GetSuccProcessor() const
{
	return succ_processor_;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ǳ�����Ϣ��������Ĭ��ʵ�֣�������������账����Ϣ������ʵ�ִ˽ӿ�
 * ��  ��: [in] msg_type ��Ϣ����
 *         [in] msg_data ��Ϣ����
 * ����ֵ: ������
 * ��  ��: 
 *   ʱ�� 2014��03��23��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class Drived, class Next, class Succ>
inline ProcInfo PktProcessor<Drived, Next, Succ>::DoProcessMsg(MsgType msg_type, void* msg_data)
{
	return PI_HAS_PROCESSED;
}

}

#endif