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
#include <boost/thread.hpp>

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���λ�����У���ǰֻ֧��һ�������ߺ�һ��������
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��01��02��
 ******************************************************************************/
template<class T>
class CircularFifo : public boost::noncopyable
{
public:
	inline CircularFifo(size_t fifo_size);

	inline virtual ~CircularFifo();

	//--------------------------------------------------------------------------

	// ��ȡ��д������
	inline T* GetWritableItem();

	// ֪ͨ����д���Ѿ����
	inline void FinishedWrite();

	// ��ȡ�ɶ�������
	inline T* GetReadableItem();

	// ֪ͨ���ݶ�ȡ�Ѿ����
	inline void FinishedRead();

	//--------------------------------------------------------------------------

	// ��ȡָ�������Ŀ�д������
	inline bool GetMultiWritableItems(uint32 size, std::vector<T*>& items);

	// ֪ͨ��������д���Ѿ����
	inline void FinishedWriteMulti(uint32 size);

	// ��ȡ���пɶ�������
	inline bool GetMultiReadableItems(std::vector<T*>& items);

	// ֪ͨ�������ݶ�ȡ�Ѿ����
	inline void FinishedReadMulti(uint32 size);

	//--------------------------------------------------------------------------

	// �Ƿ��޿ɶ�������
	inline bool IsEmpty() const;

private:

	// ������/д�±�
	inline size_t Increment(size_t idx) const;

	// ָ������������/д�±�
	inline size_t IncrementN(size_t index, size_t n) const;

	// �Ƿ��޿�д������
	inline bool IsFull() const;

	// �Ƿ����㹻�Ŀռ����洢ָ������Ԫ��
	inline bool HasEnoughSpace(uint32 size) const;

	// lock-free���
	inline bool IsLockFree() const;

private:
	// ��βָ��д��Ԫ��
	std::atomic<size_t> tail_;

	// ��ͷָ���ȡԪ��
	std::atomic<size_t> head_;

	// ���λ���������̬����
	T* element_array_;

	// ���λ���������
	size_t fifo_size_;
};

}

#include "circular_fifo-inl.hpp"

#endif

