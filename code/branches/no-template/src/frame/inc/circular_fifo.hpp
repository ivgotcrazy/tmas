/*#############################################################################
 * 文件名   : circular_fifo.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月19日
 * 文件描述 : CircularFifo类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CIRCULAR_FIFO
#define BROADINTER_CIRCULAR_FIFO

#include <atomic>
#include <cstddef>

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 环形缓冲队列，一个生产者和多个消费者
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
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
	// 从尾指针写入元素
	std::atomic<size_t> tail_;

	// 从头指针读取元素
	std::atomic<size_t> head_;

	// 环形缓冲队列
	Element* element_array_;
	size_t fifo_size_;

	// 用户缓冲数据的创建和销毁回调接口
	ItemConstructor constructor_;
	ItemDestructor destructor_;
};

}

#include "circular_fifo-inl.hpp"

#endif

