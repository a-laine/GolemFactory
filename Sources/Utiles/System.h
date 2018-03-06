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

#elif defined(__APPLE__) && defined(__MACH__)
    #define GF_OS_MACOS

#elif defined(__linux__)
    #define GF_OS_LINUX

#else
    #error Operating system not supported

#endif