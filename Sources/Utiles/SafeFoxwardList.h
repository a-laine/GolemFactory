#pragma once

#include <atomic>








template<class T> class SafeFowardList
{
	private:
		template<class T> class Node
		{
			public:
				Node() : element(), next(nullptr), thread(0) {}
				Node(T value) : element(value), next(nullptr), thread(0) {}
				Node(const T& value) : element(value), next(nullptr), thread(0) {}

				T element;
				std::atomic<Node<T>* > next;
				std::atomic_uint thread;
		};

		template<class T, class Ref, class Ptr> struct SafeFowardListIterator
		{
			//	Typedef
			typedef SafeFowardListIterator<T, T&, T*>             iterator;
			typedef SafeFowardListIterator<T, const T&, const T*> const_iterator;
			typedef SafeFowardListIterator<T, Ref, Ptr>           Self;
			typedef T value_type;
			typedef Ptr pointer;
			typedef Ref reference;
			//

			//	Default
			SafeFowardListIterator(Node<T>* x = nullptr) : itNode(x)
			{
				if(itNode) itNode->thread++;
			}
			SafeFowardListIterator() : itNode(nullptr) {}
			SafeFowardListIterator(const SafeFowardListIterator& x) : itNode(x.itNode)
			{
				if (itNode) itNode->thread++;
			}
			~SafeFowardListIterator()
			{
				if (itNode) itNode->thread--;
			}
			//

			//	Operator overload
			bool operator==(const SafeFowardListIterator& x) const
			{
				return itNode == x.itNode;
			}
			bool operator!=(const SafeFowardListIterator& x) const
			{
				return itNode != x.itNode;
			}
			reference operator*() const 
			{
				return itNode->element;
			}
			pointer operator->() const
			{
				return &(operator*());
			}
			Self& operator++()
			{
				Node<T>* nextptr = itNode->next.load();
				nextptr->thread++;
				itNode->thread--;
				itNode = nextptr;
				return *this;
			}
			Self& operator++(int)
			{
				return ++(*this);
			}
			//

			//	Attributes
			Node<T>* itNode;
			//
		};

	public:
		//	Typedef
		typedef SafeFowardListIterator<T, T&, T*>             iterator;
		typedef SafeFowardListIterator<T, const T&, const T*> const_iterator;
		typedef T											  value_type;
		//

		//	Default
		SafeFowardList()
		{
			head = new Node<T>();
			head->next = nullptr;
		}
		~SafeFowardList()
		{
			clear();
			// TODO : delete head;
		}
		//

		//	Iterators
		iterator before_begin()
		{
			return head;
		}
		const_iterator cbefore_begin()
		{
			return head;
		}
		iterator begin()
		{
			return head->next.load();
		}
		const_iterator cbegin() const
		{
			return head->next.load();
		}
		iterator end()
		{
			return nullptr;
		}
		const_iterator cend() const
		{
			return nullptr;
		}
		//

		//	Element access
		T& front()
		{
			return *begin();
		}
		const T& front() const
		{
			return *begin();
		}
		//

		//	Modifiers

		iterator insert_after(iterator position,const value_type& value)
		{
			if (position != end())
			{
				Node<T>* newbie = new Node<T>(x);
				newbie->next = position.itNode->next.load();
				position.itNode->next = newbie;
			}
			return position;
		}
		iterator erase_after(iterator position)
		{
			if (position != end() && position++ != end())
			{
				iterator erased = position++;
				iterator newnext = erased++;
				position.itNode->next = newnext.itNode;
				//TODO : delete erased->itNode;
				return newnext;
			}
			else return end();
		}
		iterator erase_after(iterator first, iterator last)
		{

		}

		void clear()
		{
			for (iterator it = before_begin(); it != end();)
				it = erase_after(it);
			head->next = nullptr;
		}
		//

		//	Capacity
		bool empty() const
		{
			return begin() == end();
		}
		size_t size() const
		{
			size_t result = 0;
			for (iterator it = begin(); it != end(); it++)
				result++;
			return result;
		}
		//

		//	Attributes
		Node<T>* head;
		//
};

