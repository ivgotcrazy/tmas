/*#############################################################################
 * 文件名   : obj_pool.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月20日
 * 文件描述 : MultiThreadObjPool类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_MEM_POOL
#define BROADINTER_MEM_POOL

#include <list>
#include <boost/noncopyable.hpp>

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 多线程对象池
 * 作  者: teck_zhou
 * 时  间: 2014年03月20日
 ******************************************************************************/
template<typename T>
class MultiThreadObjPool : public boost::noncopyable
{
public:
	MultiThreadObjPool(size_t pool_size = EXPAND_SIZE);
	~MultiThreadObjPool();

	inline T* AllocObj();
	inline void FreeObj(T* obj);

private:
	enum { EXPAND_SIZE = 128 };

	inline void Expand(size_t pool_size = EXPAND_SIZE);

private:
	std::list<T*> obj_list_;

	boost::mutex pool_mutex_;
};

/*-----------------------------------------------------------------------------
 * 描  述: 构造函数
 * 参  数: [in] pool_size 对象池初始大小
 * 返回值: 
 * 修  改:
 *   时间 2014年03月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<typename T>
MultiThreadObjPool<T>::MultiThreadObjPool(size_t pool_size /* = EXPAND_SIZE */)
{
	Expand(pool_size);
}

/*-----------------------------------------------------------------------------
 * 描  述: 析构函数
 * 参  数: 
 * 返回值: 
 * 修  改:
 *   时间 2014年03月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<typename T>
MultiThreadObjPool<T>::~MultiThreadObjPool()
{
	while (!obj_list_.empty())
	{
		delete obj_list_.front();
		obj_list_.pop_front();
	}
}

/*-----------------------------------------------------------------------------
 * 描  述: 申请对象
 * 参  数: 
 * 返回值: 对象指针
 * 修  改:
 *   时间 2014年03月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<typename T>
inline T* MultiThreadObjPool<T>::AllocObj()
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	if (obj_list_.empty())
	{
		Expand();
		LOG(WARNING) << "Expand object pool | size: " << obj_list_.size();
	}

	T* obj = obj_list_.front();
	obj_list_.pop_front();

	return obj;
}

/*-----------------------------------------------------------------------------
 * 描  述: 释放对象
 * 参  数: [in] obj 待释放对象指针
 * 返回值: 
 * 修  改:
 *   时间 2014年03月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<typename T>
void MultiThreadObjPool<T>::FreeObj(T* obj)
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	obj_list_.push_back(obj);
}

/*-----------------------------------------------------------------------------
 * 描  述: 扩充对象池大小
 * 参  数: [in] expand_size 扩充大小
 * 返回值: 
 * 修  改:
 *   时间 2014年03月20日
 *   作者 teck_zhou
 *   描述 创建
 ----------------------------------------------------------------------------*/
template<typename T>
void MultiThreadObjPool<T>::Expand(size_t expand_size /* = EXPAND_SIZE */)
{
	for (size_t i = 0; i < expand_size; i++)
	{
		obj_list_.push_back(new T());
	}
}

}

#endif