/*#############################################################################
 * �ļ���   : circular_fifo-inl.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��19��
 * �ļ����� : CircularFifo��ʵ��
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CIRCULAR_FIFO_INL
#define BROADINTER_CIRCULAR_FIFO_INL

#include "circular_fifo.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * ��  ��: ���캯��
 * ��  ��: [in] constructor ���ݹ�����
 *         [in] destructor ����������
 *         [in] fifo_size ������г���
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
CircularFifo::CircularFifo(ItemConstructor constructor, 
	ItemDestructor destructor, size_t fifo_size) 
	: tail_(0)
	, head_(0)
	, fifo_size_(fifo_size)
	, constructor_(constructor)
	, destructor_(destructor)
{
	if (fifo_size == 0)
	{
		fifo_size = 9;
	}

	element_array_ = new Element[fifo_size];

	for (size_t i = 0; i < fifo_size; i++)
	{
		element_array_[i].state = ES_WRITABLE;
		element_array_[i].data  = constructor_();
	}
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
CircularFifo::~CircularFifo()
{
	for (size_t i = 0; i < fifo_size_; i++)
	{
		destructor_(element_array_[i].data);
	}

	delete[] element_array_;
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ��д������
 * ��  ��: [out] item ������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool CircularFifo::GetWritableItem(void** item)
{	
	if (IsFull()) return false;

	const auto current_tail = tail_.load();

	const auto state = element_array_[current_tail].state.load();

	if (state != ES_WRITABLE) return false; 

	TMAS_ASSERT(state == ES_WRITABLE);

	*item = element_array_[current_tail].data;

	element_array_[current_tail].state.store(ES_WRITING);

	return true;
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
void CircularFifo::FinishedWrite()
{
	const auto current_tail = tail_.load();

	const auto state = element_array_[current_tail].state.load();

	TMAS_ASSERT(state == ES_WRITING);

	element_array_[current_tail].state.store(ES_READABLE);
	
	tail_.store(Increment(current_tail));
}

/*-----------------------------------------------------------------------------
 * ��  ��: ��ȡ�ɶ�������
 * ��  ��: [out] item ������
 *         [out] index �������ڻ�����������
 * ����ֵ: �ɹ�/ʧ��
 * ��  ��:
 *   ʱ�� 2014��03��19��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
bool CircularFifo::GetReadableItem(void** item, size_t& index)
{
	if (IsEmpty()) return false;

	const auto current_head = head_.load();

	// ����һ��ԭ�Ӳ���
	const auto state = element_array_[current_head].state.load();

	if (state != ES_READABLE) return false;

	*item = element_array_[current_head].data;
	index = current_head;

	element_array_[current_head].state.store(ES_READING);

	head_.store(Increment(current_head)); 

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
void CircularFifo::FinishedRead(size_t index)
{
	TMAS_ASSERT(element_array_[index].state.load() == ES_READING);

	element_array_[index].state.store(ES_WRITABLE);
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
bool CircularFifo::IsEmpty() const
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
bool CircularFifo::IsFull() const
{
	const auto next_tail = Increment(tail_.load());
	return (next_tail == head_.load());
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
bool CircularFifo::IsLockFree() const
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
size_t CircularFifo::Increment(size_t index) const
{
	return (index + 1) % fifo_size_;
}

}

#endif