/*#############################################################################
 * 文件名   : object_pool.hpp
 * 创建人   : teck_zhou	
 * 创建时间 : 2014年03月20日
 * 文件描述 : ObjectPool类声明
 * 版权声明 : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_OBJECT_POOL
#define BROADINTER_OBJECT_POOL

#include <list>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace BroadInter
{

/*******************************************************************************
 * 描  述: 多线程对象池
 * 作  者: teck_zhou
 * 时  间: 2014年03月20日
 ******************************************************************************/
template<typename T>
class ObjectPool : public boost::noncopyable
				 , public boost::enable_shared_from_this<ObjectPool<T> >
{
public:
	typedef boost::shared_ptr<T> TSP;
	typedef boost::function<T*()> ObjectConstructor;
	typedef boost::function<void(T*)> ObjectDestructor;
	typedef boost::function<void(T*)> ObjectReinitializer;

	ObjectPool(ObjectConstructor constructor, 
			   ObjectDestructor destructor,
			   ObjectReinitializer reinitializer, 
			   size_t pool_size = EXPAND_SIZE);

	~ObjectPool();

	inline TSP AllocObject();
	
private:
	enum { EXPAND_SIZE = 128 * 128 };

	inline void Expand(size_t pool_size = EXPAND_SIZE);

	inline void FreeObject(T* object);

private:
	std::list<T*> obj_list_;

	boost::mutex pool_mutex_;

	ObjectConstructor constructor_;

	ObjectDestructor destructor_;

	ObjectReinitializer reinitializer_;
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
ObjectPool<T>::ObjectPool(ObjectConstructor constructor, 
	ObjectDestructor destructor, 
	ObjectReinitializer reinitializer,
	size_t pool_size /* = EXPAND_SIZE */)
	: constructor_(constructor)
	, destructor_(destructor)
	, reinitializer_(reinitializer)
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
ObjectPool<T>::~ObjectPool()
{
	while (!obj_list_.empty())
	{
		destructor_(obj_list_.front());
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
inline typename ObjectPool<T>::TSP ObjectPool<T>::AllocObject()
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	if (obj_list_.empty())
	{
		Expand();
		DLOG(WARNING) << "Expand object pool | size: " << obj_list_.size();
	}

	T* obj = obj_list_.front();
	obj_list_.pop_front();

	//DLOG(WARNING) << "Alloc | object pool size : " << obj_list_.size();

	return TSP(obj, boost::bind(&ObjectPool<T>::FreeObject, ObjectPool<T>::shared_from_this(), _1));
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
void ObjectPool<T>::FreeObject(T* object)
{
	reinitializer_(object);

	boost::mutex::scoped_lock lock(pool_mutex_);

	obj_list_.push_back(object);

	//DLOG(WARNING) << "Free | object pool size : " << obj_list_.size();
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
void ObjectPool<T>::Expand(size_t expand_size /* = EXPAND_SIZE */)
{
	for (size_t i = 0; i < expand_size; i++)
	{
		obj_list_.push_back(constructor_());
	}
}

}

#endif