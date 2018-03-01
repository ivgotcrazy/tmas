/*#############################################################################
 * �ļ���   : obj_pool.hpp
 * ������   : teck_zhou	
 * ����ʱ�� : 2014��03��20��
 * �ļ����� : MultiThreadObjPool������
 * ��Ȩ���� : Copyright (c) 2014 BroadInter. All rights reserved.
 * ##########################################################################*/

#ifndef BROADINTER_MEM_POOL
#define BROADINTER_MEM_POOL

#include <list>
#include <boost/noncopyable.hpp>

namespace BroadInter
{

/*******************************************************************************
 * ��  ��: ���̶߳����
 * ��  ��: teck_zhou
 * ʱ  ��: 2014��03��20��
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
 * ��  ��: ���캯��
 * ��  ��: [in] pool_size ����س�ʼ��С
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<typename T>
MultiThreadObjPool<T>::MultiThreadObjPool(size_t pool_size /* = EXPAND_SIZE */)
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
MultiThreadObjPool<T>::~MultiThreadObjPool()
{
	while (!obj_list_.empty())
	{
		delete obj_list_.front();
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
 * ��  ��: �ͷŶ���
 * ��  ��: [in] obj ���ͷŶ���ָ��
 * ����ֵ: 
 * ��  ��:
 *   ʱ�� 2014��03��20��
 *   ���� teck_zhou
 *   ���� ����
 ----------------------------------------------------------------------------*/
template<typename T>
void MultiThreadObjPool<T>::FreeObj(T* obj)
{
	boost::mutex::scoped_lock lock(pool_mutex_);

	obj_list_.push_back(obj);
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
void MultiThreadObjPool<T>::Expand(size_t expand_size /* = EXPAND_SIZE */)
{
	for (size_t i = 0; i < expand_size; i++)
	{
		obj_list_.push_back(new T());
	}
}

}

#endif