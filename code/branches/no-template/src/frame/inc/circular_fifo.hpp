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

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���λ�����У�һ�������ߺͶ��������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
class CircularFifo
{
public:
	typedef boost::function<void*()> ItemConstructor;
	typedef boost::function<void(void*)> ItemDestructor;

public:
	inline CircularFifo(ItemConstructor constructor, 
						ItemDestructor destructor, size_t fifo_size);
	inline virtual ~CircularFifo();

	inline bool GetWritableItem(void** item);
	inline void FinishedWrite();

	inline bool GetReadableItem(void** item, size_t& index);
	inline void FinishedRead(size_t index);

private:

	enum ElementState
	{
		ES_WRITABLE,
		ES_WRITING,
		ES_READABLE,
		ES_READING
	};

	struct Element
	{
		std::atomic<uint8> state;
		void* data;
	};

private:
	inline size_t Increment(size_t idx) const;
	inline bool IsEmpty() const;
	inline bool IsFull() const;
	inline bool IsLockFree() const;

private:
	// ��βָ��д��Ԫ��
	std::atomic<size_t> tail_;

	// ��ͷָ���ȡԪ��
	std::atomic<size_t> head_;

	// ���λ������
	Element* element_array_;
	size_t fifo_size_;

	// �û��������ݵĴ��������ٻص��ӿ�
	ItemConstructor constructor_;
	ItemDestructor destructor_;
};

}

#include "circular_fifo-inl.hpp"

#endif

