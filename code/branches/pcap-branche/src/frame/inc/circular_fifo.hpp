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

#include <boost/noncopyable.hpp>

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 环形缓冲队列，当前只支持一个生产者和多个消费者
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
class CircularFifo : public boost::noncopyable
{
public:
	// 循环缓冲自定义数据的创建接口类型
	typedef boost::function<void*()> ItemConstructor;

	// 循环缓冲自定义数据的销毁接口类型
	typedef boost::function<void(void*)> ItemDestructor;

public:
	inline CircularFifo(ItemConstructor constructor, 
		                ItemDestructor destructor, 
						size_t fifo_size);

	inline virtual ~CircularFifo();

	// 获取可写数据项
	inline bool GetWritableItem(void** item);

	// 通知FIFO数据写入已经完成
	inline void FinishedWrite();

	// 获取可读数据项
	inline bool GetReadableItem(void** item, size_t& index);

	// 通知FIFO数据读取已经完成
	inline void FinishedRead(size_t index);

private:

	enum ElementState
	{
		ES_WRITABLE,	// 可写
		ES_WRITING,		// 正在写入
		ES_READABLE,	// 可读
		ES_READING		// 正在读取
	};

	struct Element
	{
		std::atomic<uint8> state;
		void* data;
	};

private:

	// 递增读/写指针
	inline size_t Increment(size_t idx) const;

	// 是否无可读数据项
	inline bool IsEmpty() const;

	// 是否无可写数据项
	inline bool IsFull() const;

	// lock-free检测
	inline bool IsLockFree() const;

private:
	// 从尾指针写入元素
	std::atomic<size_t> tail_;

	// 从头指针读取元素
	std::atomic<size_t> head_;

	// 环形缓冲区，动态创建
	Element* element_array_;

	// 环形缓冲区数据项数量
	size_t fifo_size_;

	// 调用者提供的用户数据创建和销毁接口
	ItemConstructor constructor_;
	ItemDestructor destructor_;
};

}

#include "circular_fifo-inl.hpp"

#endif

