/*#############################################################################
 * �ļ���   : circular_fifo-inl.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��19��
 * �ļ����� : CircularFifo��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CIRCULAR_FIFO_INL
#define BROADINTER_CIRCULAR_FIFO_INL

#include <glog/logging.h>

#include "circular_fifo.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] fifo_size ������г���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
CircularFifo<T>::CircularFifo(size_t fifo_size) 
	: tail_(0), head_(0), fifo_size_(fifo_size)
{
	if (fifo_size == 0)
	{
		LOG(WARNING) << "FIFO size is auto-adjusted to 64";
		fifo_size_ = 64;
	}

	element_array_ = new T[fifo_size_];

	if (IsLockFree())
		LOG(INFO) << "Lock-free circular FIFO";
	else
		LOG(WARNING) << "Not lock-free circular FIFO";
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
CircularFifo<T>::~CircularFifo()
{
	delete[] element_array_;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ��д������
 * ��  ��: 
 * ����ֵ: ��д������ָ�룬nullptr��ʾʧ��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline T* CircularFifo<T>::GetWritableItem()
{	
	if (IsFull()) return nullptr;

	const auto current_tail = tail_.load();

	return &(element_array_[current_tail]);
}

/*-----------------------------------------------------------------------------
 * ��  ��: д������ɣ�����ֻ֧��һ�������ߣ����û�в���
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedWrite()
{
	const auto current_tail = tail_.load();
	
	tail_.store(Increment(current_tail));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡָ�������Ŀ�д������
 * ��  ��: [in] size ָ������
 *         [out] items Ԫ������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::GetMultiWritableItems(uint32 size, std::vector<T*>& items)
{
	if (!HasEnoughSpace(size)) return false;

	auto current_tail = tail_.load();

	for (uint32 i = 0; i < size; i++)
	{
		items.push_back(&(element_array_[current_tail]));
		current_tail = Increment(current_tail);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ����д�������ɴ���
 * ��  ��: [in] size д��Ԫ�ظ���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedWriteMulti(uint32 size)
{
	const auto current_tail = tail_.load();

	tail_.store(IncrementN(current_tail, size));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ�ɶ�������
 * ��  ��: 
 * ����ֵ: �ɶ�������ָ�룬nullptr��ʾʧ��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline T* CircularFifo<T>::GetReadableItem()
{
	if (IsEmpty()) return nullptr;

	const auto current_head = head_.load();

	return &(element_array_[current_head]);
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ���пɶ�������
 * ��  ��: [out] items ����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::GetMultiReadableItems(std::vector<T*>& items)
{
	if (IsEmpty()) return false;

	auto cur_head = head_.load();
	auto cur_tail = tail_.load();

	auto diff = 0;
	if (cur_tail > cur_head)
	{
		diff = cur_tail - cur_head;
	}
	else
	{
		diff = fifo_size_ - (cur_head - cur_tail);
	}

	for (uint32 i = 0; i < diff; i++)
	{
		items.push_back(&(element_array_[cur_head]));
		cur_head = Increment(cur_head);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ���������
 * ��  ��: [in] index ����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedRead()
{
	const auto current_head = head_.load();

	head_.store(Increment(current_head));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ������ȡ������ɴ���
 * ��  ��: [in] size ������ȡ����
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedReadMulti(uint32 size)
{
	const auto current_head = head_.load();

	head_.store(IncrementN(current_head, size));
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ƿ�Ϊ��
 * ��  ��: 
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::IsEmpty() const
{
	return (head_.load() == tail_.load());
}

/*-----------------------------------------------------------------------------
 * ��  ��: �������Ƿ�����
 * ��  ��: 
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::IsFull() const
{
	const auto next_tail = Increment(tail_.load());
	return (next_tail == head_.load());
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ����㹻�Ŀռ����洢ָ������Ԫ��
 * ��  ��: [in] size ָ������
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::HasEnoughSpace(uint32 size) const
{
	TMAS_ASSERT(size <= fifo_size_);

	const auto cur_tail = tail_.load();
	const auto cur_head = head_.load();

	if (IsFull()) return false;

	if (cur_tail > cur_head)
	{
		return (cur_tail - cur_head > size);
	}
	else
	{
		return (fifo_size_ - (cur_head - cur_tail) > size);
	}
}

/*-----------------------------------------------------------------------------
 * ��  ��: �Ƿ���lock-free��
 * ��  ��: 
 * ����ֵ: ��/��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::IsLockFree() const
{
	return (tail_.is_lock_free() && head_.is_lock_free());
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ƶ�����
 * ��  ��: [in] index ����
 * ����ֵ: �ƶ��������ֵ
 * ��  ��: 
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline size_t CircularFifo<T>::Increment(size_t index) const
{
	return (index + 1) % fifo_size_;
}

/*-----------------------------------------------------------------------------
 * ��  ��: �ƶ��±�ָ������
 * ��  ��: [in] index �±�
 *         [in] n ָ������ 
 * ����ֵ: �ƶ�����±�
 * ��  ��:
 *   ʱ�� 2014��04��29��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<class T>
inline size_t CircularFifo<T>::IncrementN(size_t index, size_t n) const
{
	return (index + n) % fifo_size_;
}

}

#endif