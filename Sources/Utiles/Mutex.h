#pragma once

/*!
*	\file Mutex.h
*	\brief Declaration of the Mutex class for plateform independant uses.
*	\author Aurelien & Thibault LAINE
*/

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
			/*!
			*	\typedef native_handle_type
			*	\brief 
			*/
			typedef CRITICAL_SECTION* native_handle_type;
			//

			//  Default
			/*!
			*  \brief Constructor
			*/
			Mutex()
			{
				InitializeCriticalSection(&_M_mutex);
			};

			/*!
			*  \brief Destructor
			*/
			~Mutex()
			{
				DeleteCriticalSection(&_M_mutex);
			};

			/*!
			*  \brief Constructor by coppy forbiden
			*/
			Mutex(const Mutex&) = delete;

			/*!
			*  \brief Assignment operator forbiden
			*/
			Mutex& operator=(const Mutex&) = delete;
			//

			//  Public functions
			/*!
			*	\brief Lock the mutex
			*/
			void lock()
			{
				EnterCriticalSection(&_M_mutex);
			}

			/*!
			*	\brief Lock the mutex if possible
			*	\return true if successfully lock, false otherwise
			*/
			bool try_lock()
			{
				return TryEnterCriticalSection(&_M_mutex) == TRUE;
			}

			/*!
			*  \brief Unlock the mutex
			*/
			void unlock()
			{
				LeaveCriticalSection(&_M_mutex);
			}
			
			/*!
			*	\brief ????????????????????????
			*	\return ???????????????????????
			*/
			native_handle_type native_handle()
			{
				return &_M_mutex;
			}
			//

		protected:
			//  Attributes
			CRITICAL_SECTION  _M_mutex;		//!< The critical section used to perform mutual exclusion
			//
	};

#else
    #include <mutex>

	/*!
	*	\typedef Mutex
	*	\brief std::mutex redefinition
	*/
    typedef std::mutex Mutex;
#endif