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
#include <boost/thread.hpp>

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 环形缓冲队列，当前只支持一个生产者和一个消费者
 * 作  者: teck_zhou
 * 时  间: 2014年01月02日
 ******************************************************************************/
template<class T>
class CircularFifo : public boost::noncopyable
{
public:
	inline CircularFifo(size_t fifo_size);

	inline virtual ~CircularFifo();

	//--------------------------------------------------------------------------

	// 获取可写数据项
	inline T* GetWritableItem();

	// 通知数据写入已经完成
	inline void FinishedWrite();

	// 获取可读数据项
	inline T* GetReadableItem();

	// 通知数据读取已经完成
	inline void FinishedRead();

	//--------------------------------------------------------------------------

	// 获取指定个数的可写数据项
	inline bool GetMultiWritableItems(uint32 size, std::vector<T*>& items);

	// 通知批量数据写入已经完成
	inline void FinishedWriteMulti(uint32 size);

	// 获取所有可读数据项
	inline bool GetMultiReadableItems(std::vector<T*>& items);

	// 通知批量数据读取已经完成
	inline void FinishedReadMulti(uint32 size);

	//--------------------------------------------------------------------------

	// 是否无可读数据项
	inline bool IsEmpty() const;

private:

	// 递增读/写下标
	inline size_t Increment(size_t idx) const;

	// 指定步数递增读/写下标
	inline size_t IncrementN(size_t index, size_t n) const;

	// 是否无可写数据项
	inline bool IsFull() const;

	// 是否有足够的空间来存储指定个数元素
	inline bool HasEnoughSpace(uint32 size) const;

	// lock-free检测
	inline bool IsLockFree() const;

private:
	// 从尾指针写入元素
	std::atomic<size_t> tail_;

	// 从头指针读取元素
	std::atomic<size_t> head_;

	// 环形缓冲区，动态创建
	T* element_array_;

	// 环形缓冲区容量
	size_t fifo_size_;
};

}

#include "circular_fifo-inl.hpp"

#endif

