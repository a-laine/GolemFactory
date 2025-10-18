#pragma once

#include "System.h"

#if defined(GF_OS_WINDOWS)

	#include <windows.h>

	/** \class Mutex
	*	\brief Mutex class for mutual exclusion.
	*
	*	On windows std::mutex is not enough efficent because of the type of object uses.
	*	For similar performance independing on plateform we reimplemented a mutex class for windows using CRITICAL_SECTION object.
	*
	*/
	class Mutex
	{
		public:
			//  Miscellaneous
			typedef CRITICAL_SECTION* native_handle_type;
			//

			// Default
			Mutex() { InitializeCriticalSection(&_M_mutex); };
			~Mutex() { DeleteCriticalSection(&_M_mutex); };

			Mutex(const Mutex&) = delete;
			Mutex& operator=(const Mutex&) = delete;
			//

			//  Public functions
			void lock() { EnterCriticalSection(&_M_mutex); }
			bool try_lock() { return TryEnterCriticalSection(&_M_mutex) == TRUE; }
			void unlock() { LeaveCriticalSection(&_M_mutex); }
			
			//????????????????????????
			native_handle_type native_handle() { return &_M_mutex; }
			//

		protected:
			//  Attributes
			CRITICAL_SECTION  _M_mutex;		//!< The critical section used to perform mutual exclusion
			//
	};

#else
    #include <mutex>

    typedef std::mutex Mutex;
#endif

class MutexGuard
{
	public:
		MutexGuard(Mutex& m) : m_mutex(&m) { m_mutex->lock(); };
		~MutexGuard() { m_mutex->unlock(); };

	protected:
		Mutex* m_mutex;
};