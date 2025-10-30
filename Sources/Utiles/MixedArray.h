#pragma once


#include <vector>

template<typename T, size_t t_stackCapacity>
class MixedArray
{
	public:
		~MixedArray()
		{
			if (m_dynamicArray)
				delete[] m_dynamicArray;
		}
		void push_back(const T& element)
		{
			if (m_staticSize < t_stackCapacity)
			{
				m_staticData[m_staticSize] = element;
				m_staticSize++;
			}
			else
			{
				if (!m_dynamicArray)
				{
					m_dynamicCapacity = 2 * t_stackCapacity;
					m_dynamicArray = new T[m_dynamicCapacity];
					memcpy(m_dynamicArray, m_staticData, sizeof(T) * t_stackCapacity);
					m_dynamicSize = m_staticSize;
				}
				else if (m_dynamicSize >= m_dynamicCapacity)
				{
					size_t newCapacity = 2 * m_dynamicCapacity;
					T* newArray = new T[newCapacity];
					memcpy(newArray, m_dynamicArray, sizeof(T) * m_dynamicSize);
					m_dynamicCapacity = newCapacity;
					delete[] m_dynamicArray;
					m_dynamicArray = newArray;
				}

				m_dynamicArray[m_dynamicSize] = element;
				m_dynamicSize++;
			}
		}
		void pop_back()
		{
			if (m_staticSize <= t_stackCapacity)
				m_staticSize--;
			else
				m_dynamicSize--;
		}
		T& back()
		{
			if (m_staticSize <= t_stackCapacity)
				return m_staticData[m_staticSize - 1];
			else
				return m_dynamicArray[m_dynamicSize - 1];
		}
		bool empty() const
		{
			if (m_staticSize < t_stackCapacity)
				return m_staticSize == 0;
			else
				return m_dynamicSize == 0;
		}
		void clear()
		{
			if (m_staticSize < t_stackCapacity)
				m_staticSize = 0;
			else
				m_dynamicSize = 0;
		}
		size_t size() const
		{
			if (m_staticSize < t_stackCapacity)
				return m_staticSize;
			else
				return m_dynamicSize;
		}
		T* data()
		{
			if (m_staticSize <= t_stackCapacity)
				return m_staticData;
			else
				return m_dynamicArray;
		}
		const T* data() const
		{
			if (m_staticSize <= t_stackCapacity)
				return m_staticData;
			else
				return m_dynamicArray;
		}

		T& operator[](int i)
		{
			if (m_staticSize < t_stackCapacity)
				return m_staticData[i];
			else
				return m_dynamicArray[i];
		}
		const T& operator[](int i) const
		{
			if (m_staticSize < t_stackCapacity)
				return m_staticData[i];
			else
				return m_dynamicArray[i];
		}


	private:
		size_t m_dynamicSize = 0;
		size_t m_dynamicCapacity = 0;
		T* m_dynamicArray = nullptr;
		size_t m_staticSize = 0;
		T m_staticData[t_stackCapacity];
};