#pragma once

#include <vector>

template<typename T>
class BankArray
{
	public:
		~BankArray()
		{
			flush();
		}

		void clear()
		{
			for (Bank* b : m_banks)
			{
				if (!std::is_trivially_destructible<T>::value)
				{
					for (int i = 0; i < 32; i++)
					{
						if (b->m_occupancy & (1 << i))
							b->m_data[i].~T();
						//b->m_data[i] = nullptr;
					}
				}
				b->m_occupancy = 0x00000000;
			}
			m_size = 0;
		}
		void flush()
		{
			for (Bank* b : m_banks)
				delete b;
			m_banks.clear();
			m_size = 0;
		}
		uint32_t size() const
		{
			return m_size;
		}
		uint32_t range() const
		{
			return 32u * m_banks.size();
		}
		bool isValid(int i)
		{
			int bankIndex = i >> 5;
			int index = i & 0b11111;
			return m_banks[bankIndex]->m_occupancy & (1 << index);
		}
		
		int add(const T& element)
		{
			// search free slot in banks
			int bankIndex = -1;
			for (int i = 0; i < m_banks.size(); i++)
			{
				if (m_banks[i]->m_occupancy != 0xFFFFFFFF)
				{
					bankIndex = i;
					break;
				}
			}

			// create new bank
			if (bankIndex < 0)
			{
				Bank* newbank = new Bank();
				newbank->m_occupancy = 0;
				bankIndex = m_banks.size();
				m_banks.push_back(newbank);
			}

			// search free slot in bank
			Bank* selectedBank = m_banks[bankIndex];
			int index = -1;
			for (int i = 0; i < 32; i++)
			{
				if ((selectedBank->m_occupancy & (1 << i)) == 0)
				{
					index = i;
					break;
				}
			}

			// insert
			if (index >= 0)
			{
				selectedBank->m_data[index] = element;
				selectedBank->m_occupancy |= (1 << index);
				m_size++;
				return (bankIndex << 5) | index;
			}
			return -1;
		}
		void remove(int i)
		{
			int bankIndex = i >> 5;
			int index = i & 0b11111;
			if (bankIndex >= m_banks.size() || (m_banks[bankIndex]->m_occupancy & (1 << index)) == 0)
				return;

			m_size--;
			if (!std::is_trivially_destructible<T>::value)
			{
				m_banks[bankIndex]->m_data[index].~T();
			}
			m_banks[bankIndex]->m_occupancy &= ~(1 << index);
		}

		T& operator[](int i)
		{
			int bankIndex = i >> 5;
			int index = i & 0b11111;
			return m_banks[bankIndex]->m_data[index];
		}
		const T& operator[](int i) const
		{
			int bankIndex = i >> 5;
			int index = i & 0b11111;
			return m_banks[bankIndex]->m_data[index];
		}

	protected:
		struct Bank
		{
			Bank() { m_occupancy = 0; }
			~Bank() 
			{
				if (!std::is_trivially_destructible<T>::value)
				{
					for (int i = 0; i < 32; i++)
					{
						if (m_occupancy & (1 << i))
							m_data[i].~T();
					}
				}
				m_occupancy = 0;
			}
			uint32_t m_occupancy;
			T m_data[32];
		};
		uint32_t m_size = 0;
		std::vector<Bank*> m_banks;
};