/*#############################################################################
 * �ļ���   : circular_fifo.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��19��
 * �ļ����� : CircularFifo������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CIRCULAR_FIFO
#define BROADINTER_CIRCULAR_FIFO

#include <atomic>
#include <cstddef>

#include <boost/noncopyable.hpp>

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���λ�����У���ǰֻ֧��һ�������ߺͶ��������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class CircularFifo : public boost::noncopyable
{
public:
	// ѭ�������Զ������ݵĴ����ӿ�����
	typedef boost::function<void*()> ItemConstructor;

	// ѭ�������Զ������ݵ����ٽӿ�����
	typedef boost::function<void(void*)> ItemDestructor;

public:
	inline CircularFifo(ItemConstructor constructor, 
		                ItemDestructor destructor, 
						size_t fifo_size);

	inline virtual ~CircularFifo();

	// ��ȡ��д������
	inline bool GetWritableItem(void** item);

	// ֪ͨFIFO����д���Ѿ����
	inline void FinishedWrite();

	// ��ȡ�ɶ�������
	inline bool GetReadableItem(void** item, size_t& index);

	// ֪ͨFIFO���ݶ�ȡ�Ѿ����
	inline void FinishedRead(size_t index);

private:

	enum ElementState
	{
		ES_WRITABLE,	// ��д
		ES_WRITING,		// ����д��
		ES_READABLE,	// �ɶ�
		ES_READING		// ���ڶ�ȡ
	};

	struct Element
	{
		std::atomic<uint8> state;
		void* data;
	};

private:

	// ������/дָ��
	inline size_t Increment(size_t idx) const;

	// �Ƿ��޿ɶ�������
	inline bool IsEmpty() const;

	// �Ƿ��޿�д������
	inline bool IsFull() const;

	// lock-free���
	inline bool IsLockFree() const;

private:
	// ��βָ��д��Ԫ��
	std::atomic<size_t> tail_;

	// ��ͷָ���ȡԪ��
	std::atomic<size_t> head_;

	// ���λ���������̬����
	Element* element_array_;

	// ���λ���������������
	size_t fifo_size_;

	// �������ṩ���û����ݴ��������ٽӿ�
	ItemConstructor constructor_;
	ItemDestructor destructor_;
};

}

#include "circular_fifo-inl.hpp"

#endif

