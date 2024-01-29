#pragma once

#include <vector>
#include <list>



template<typename T>
class ObjectPool
{
	public:
		ObjectPool() {};
		virtual ~ObjectPool()
		{
			m_banks.clear();
			m_freeObjects.clear();
		}


		T* getFreeObject()
		{
			if (!m_freeObjects.empty())
			{
				T* obj = m_freeObjects.back();
				m_freeObjects.pop_back();
				return obj;
			}

			m_banks.emplace_back(ObjectBank());
			auto& objArray = m_banks.back().m_objects;
			objArray.resize(64);
			for (T& element : objArray)
				m_freeObjects.push_back(&element);

			T* obj = m_freeObjects.back();
			m_freeObjects.pop_back();
			return obj;
		}

		void releaseObject(T* object)
		{
			m_freeObjects.push_back(object);
		}





	protected:
		struct ObjectBank
		{
			std::vector<T> m_objects;
		};
		std::list<ObjectBank> m_banks;
		std::vector<T*> m_freeObjects;
};



