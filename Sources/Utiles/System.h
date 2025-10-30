#pragma once

/*!
*	\file System.h
*	\brief Golem Factory OS macro definition,
*	\author Aurelien LAINE
*/


#if   defined(_WIN32)
    #define GF_OS_WINDOWS
	#define NOMINMAX		//!< if not declared on windows plateforms collision occur with STL min and max functions

	#include <windows.h>

    #define GF_FUNCTION __FUNCSIG__

#elif defined(__APPLE__) && defined(__MACH__)
    #define GF_OS_MACOS

    #define GF_FUNCTION __func__

#elif defined(__linux__)
    #define GF_OS_LINUX

    #define GF_FUNCTION __PRETTY_FUNCTION__

#else
    #error Operating system not supported

#endif