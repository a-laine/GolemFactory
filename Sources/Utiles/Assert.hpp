#pragma once


#ifndef NDEBUG
#define GF_ASSERT_ON
#endif // !NDEBUG



#ifdef GF_ASSERT_ON
#define GF_ASSERT(...) GF_ASSERT_VARIADIC_IMPL(__VA_ARGS__, "")
#else
#define GF_ASSERT(...) ((void) 0)
#endif // GF_ASSERT_ON

#define GF_ASSERT_VARIADIC_IMPL(condition, message, ...) GF_ASSERT_IMPL(condition, __FILE__, __LINE__, message)
#define GF_ASSERT_IMPL(condition, file, line, message) ((condition) ? ((void) 0) : (*((int*) 0) = 0))

