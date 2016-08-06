#pragma once

#include "System.h"

#if defined(GF_OS_WINDOWS)

#include <windows.h>

class Mutex
{
    public:
        //  Miscellaneous
        typedef CRITICAL_SECTION* native_handle_type;
        //

        //  Default
		Mutex()
		{
			InitializeCriticalSection(&_M_mutex);
		};
		~Mutex()
		{
			DeleteCriticalSection(&_M_mutex);
		};
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;
        //

        //  Public functions
        void lock()
		{
			EnterCriticalSection(&_M_mutex);
		}
        bool try_lock()
		{
			return TryEnterCriticalSection(&_M_mutex) == TRUE;
		}
        void unlock()
		{
			LeaveCriticalSection(&_M_mutex);
		}
        native_handle_type native_handle()
		{
			return &_M_mutex;
		}
        //

    protected:
        //  Attributes
        CRITICAL_SECTION  _M_mutex;
        //
};

#else
    #include <mutex>
    typedef std::mutex Mutex;
#endif