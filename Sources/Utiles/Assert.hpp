#pragma once

//#define GF_ASSERT_ON


#ifdef GF_ASSERT_ON

#include <cassert>
#include <iostream>

#if   defined(_WIN32)
    #ifndef NDEBUG
        #define GF_ASSERT_IMPL(condition, file, line, message) \
        do { \
            if ((!(condition)) && (1 == _CrtDbgReport(_CRT_ERROR,file,line,NULL,"%s",message)))\
                __debugbreak(); \
        } while (0)
    #else
    #define GF_ASSERT_IMPL(condition, file, line, message) \
        do { \
            if (!(condition)){\
                std::cout<<"ASSERT FAIL : " << #condition << std::endl;\
                std::cout<<"       file : " << file << std::endl;\
                std::cout<<"       line : " << line << std::endl;\
                std::cout<<"    message : " << message << std::endl;\
                __debugbreak();}\
        } while (0)
    #endif
#else
    #define GF_ASSERT_IMPL(condition, file, line, message) assert(condition && message)

#endif


#define GF_ASSERT_MSG(condition, message,...) GF_ASSERT_IMPL(condition,__FILE__,__LINE__,message)
#define GF_ASSERT(condition,...) GF_ASSERT_MSG(condition,"(no message)",__FILE__,__LINE__)

#else
#define GF_ASSERT_MSG(...) ((void) 0)
#define GF_ASSERT(...) ((void) 0)
#endif // GF_ASSERT_ON


