/*#############################################################################
 * 文件名   : circular_fifo-inl.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月19日
 * 文件描述 : CircularFifo类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CIRCULAR_FIFO_INL
#define BROADINTER_CIRCULAR_FIFO_INL

#include <glog/logging.h>

#include "circular_fifo.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] fifo_size 缓冲队列长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
CircularFifo<T>::~CircularFifo()
{
	delete[] element_array_;
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取可写数据项
 * 参  数: 
 * 返回值: 可写数据项指针，nullptr表示失败
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline T* CircularFifo<T>::GetWritableItem()
{	
	if (IsFull()) return nullptr;

	const auto current_tail = tail_.load();

	return &(element_array_[current_tail]);
}

/*-----------------------------------------------------------------------------
 * 描  述: 写操作完成，由于只支持一个生产者，因此没有参数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedWrite()
{
	const auto current_tail = tail_.load();
	
	tail_.store(Increment(current_tail));
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取指定个数的可写数据项
 * 参  数: [in] size 指定个数
 *         [out] items 元素容器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 批量写入操作完成处理
 * 参  数: [in] size 写入元素个数
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedWriteMulti(uint32 size)
{
	const auto current_tail = tail_.load();

	tail_.store(IncrementN(current_tail, size));
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取可读数据项
 * 参  数: 
 * 返回值: 可读数据项指针，nullptr表示失败
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline T* CircularFifo<T>::GetReadableItem()
{
	if (IsEmpty()) return nullptr;

	const auto current_head = head_.load();

	return &(element_array_[current_head]);
}

/*-----------------------------------------------------------------------------
 * 描  述: 获取所有可读数据项
 * 参  数: [out] items 数据项容器
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 读操作完成
 * 参  数: [in] index 数据项索引
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedRead()
{
	const auto current_head = head_.load();

	head_.store(Increment(current_head));
}

/*-----------------------------------------------------------------------------
 * 描  述: 批量读取操作完成处理
 * 参  数: [in] size 批量读取个数
 * 返回值: 
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline void CircularFifo<T>::FinishedReadMulti(uint32 size)
{
	const auto current_head = head_.load();

	head_.store(IncrementN(current_head, size));
}

/*-----------------------------------------------------------------------------
 * 描  述: 缓冲区是否为空
 * 参  数: 
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::IsEmpty() const
{
	return (head_.load() == tail_.load());
}

/*-----------------------------------------------------------------------------
 * 描  述: 缓冲区是否已满
 * 参  数: 
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::IsFull() const
{
	const auto next_tail = Increment(tail_.load());
	return (next_tail == head_.load());
}

/*-----------------------------------------------------------------------------
 * 描  述: 是否有足够的空间来存储指定个数元素
 * 参  数: [in] size 指定个数
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 是否是lock-free的
 * 参  数: 
 * 返回值: 是/否
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline bool CircularFifo<T>::IsLockFree() const
{
	return (tail_.is_lock_free() && head_.is_lock_free());
}

/*-----------------------------------------------------------------------------
 * 描  述: 移动索引
 * 参  数: [in] index 索引
 * 返回值: 移动后的索引值
 * 修  改: 
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline size_t CircularFifo<T>::Increment(size_t index) const
{
	return (index + 1) % fifo_size_;
}

/*-----------------------------------------------------------------------------
 * 描  述: 移动下标指定步数
 * 参  数: [in] index 下标
 *         [in] n 指定步数 
 * 返回值: 移动后的下标
 * 修  改:
 *   时间 2014年04月29日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<class T>
inline size_t CircularFifo<T>::IncrementN(size_t index, size_t n) const
{
	return (index + n) % fifo_size_;
}

}

#endif