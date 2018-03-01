/*#############################################################################
 * �ļ���   : object_pool.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��20��
 * �ļ����� : ObjectPool������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_OBJECT_POOL
#define BROADINTER_OBJECT_POOL

#include <list>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���̶߳����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��20��
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
 * ��  ��: ���캯��
 * ��  ��: [in] pool_size ����س�ʼ��С
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: ��������
 * ��  ��: 
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �������
 * ��  ��: 
 * ����ֵ: ����ָ��
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �ͷŶ���
 * ��  ��: [in] obj ���ͷŶ���ָ��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
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
 * ��  ��: �������ش�С
 * ��  ��: [in] expand_size �����С
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
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