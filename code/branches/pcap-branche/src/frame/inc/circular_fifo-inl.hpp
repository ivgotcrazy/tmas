/*#############################################################################
 * 文件名   : circular_fifo-inl.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月19日
 * 文件描述 : CircularFifo类实现
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_CIRCULAR_FIFO_INL
#define BROADINTER_CIRCULAR_FIFO_INL

#include "circular_fifo.hpp"
#include "tmas_assert.hpp"

namespace BroadInter
{

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] constructor 数据构造器
 *         [in] destructor 数据析构器
 *         [in] fifo_size 缓冲队列长度
 * 返回值: 
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 获取可写数据项
 * 参  数: [out] item 数据项
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 写操作完成，由于只支持一个生产者，因此没有参数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
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
 * 描  述: 获取可读数据项
 * 参  数: [out] item 数据项
 *         [out] index 数据项在缓冲区的索引
 * 返回值: 成功/失败
 * 修  改:
 *   时间 2014年03月19日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
bool CircularFifo::GetReadableItem(void** item, size_t& index)
{
	if (IsEmpty()) return false;

	const auto current_head = head_.load();

	// 这是一个原子操作
	const auto state = element_array_[current_head].state.load();

	if (state != ES_READABLE) return false;

	*item = element_array_[current_head].data;
	index = current_head;

	element_array_[current_head].state.store(ES_READING);

	head_.store(Increment(current_head)); 

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
void CircularFifo::FinishedRead(size_t index)
{
	TMAS_ASSERT(element_array_[index].state.load() == ES_READING);

	element_array_[index].state.store(ES_WRITABLE);
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
bool CircularFifo::IsEmpty() const
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
bool CircularFifo::IsFull() const
{
	const auto next_tail = Increment(tail_.load());
	return (next_tail == head_.load());
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
bool CircularFifo::IsLockFree() const
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
size_t CircularFifo::Increment(size_t index) const
{
	return (index + 1) % fifo_size_;
}

}

#endif