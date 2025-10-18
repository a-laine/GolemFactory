#pragma once

#include <mutex>
#include <atomic>

template <typename T, size_t t_capacity>
class ThreadSafeRingBuffer
{
	public:
		bool push_back(const T& item)
		{
			bool result = false;
			m_lock.lock();
			int next = (m_head + 1) % t_capacity;
			if (next != m_tail)
			{
				m_data[m_head] = item;
				m_head = next;
				result = true;
			}
			m_lock.unlock();
			return result;
		}
		bool pop_front(T& item)
		{
			bool result = false;
			m_lock.lock();
			if (m_tail != m_head)
			{
				item = m_data[m_tail];
				m_tail = (m_tail + 1) % t_capacity;
				result = true;
			}
			m_lock.unlock();
			return result;
		}

	private:
		T m_data[t_capacity];
		int m_head = 0;
		int m_tail = 0;
		std::mutex m_lock;
};

