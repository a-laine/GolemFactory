#pragma once

// OS Macro definition
#if   defined(_WIN32)
    #define GF_OS_WINDOWS
	#define NOMINMAX

#elif defined(__APPLE__) && defined(__MACH__)
    #define GF_OS_MACOS

#elif defined(__linux__)
    #define GF_OS_LINUX

#else
    #error Operating system not supported

#endif