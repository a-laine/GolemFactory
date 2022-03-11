#pragma once


#include <inttypes.h>


template <typename T> class Tvec2;
template <typename T> class Tvec3;
template <typename T> class Tvec4;

typedef Tvec2<float>		vec2f;
typedef Tvec2<double>		vec2d;
typedef Tvec2<int32_t>		vec2i;
typedef Tvec2<uint32_t>		vec2ui;
typedef Tvec2<bool>			vec2b;

typedef Tvec3<float>		vec3f;
typedef Tvec3<double>		vec3d;
typedef Tvec3<int32_t>		vec3i;
typedef Tvec3<uint32_t>		vec3ui;
typedef Tvec3<bool>			vec3b;

typedef Tvec4<float>		vec4f;
typedef Tvec4<double>		vec4d;
typedef Tvec4<int32_t>		vec4i;
typedef Tvec4<uint32_t>		vec4ui;
typedef Tvec4<bool>			vec4b;


#ifdef _DEBUG
	#undef USE_SIMD

#elif defined(__AVX2__)
	#include <xmmintrin.h>
	#include <emmintrin.h>
	#include <immintrin.h>

	#define USE_SIMD   //GLM_ARCH_AVX2 | GLM_ARCH_AVX | GLM_ARCH_SSE4 | GLM_ARCH_SSE3 | GLM_ARCH_SSE2
	#define USE_AVX2

#elif defined(__AVX__)
	#include <xmmintrin.h>
	#include <emmintrin.h>
	#include <immintrin.h>
	#include <intrin.h>

	#define USE_SIMD   //GLM_ARCH_AVX | GLM_ARCH_SSE4 | GLM_ARCH_SSE3 | GLM_ARCH_SSE2
	#define USE_AVX

#elif _M_IX86_FP == 2
	#include <xmmintrin.h>
	#include <emmintrin.h>
	#include <immintrin.h>
	#include <intrin.h>

	#define USE_SIMD   //GLM_ARCH_SSE2
	#define USE_SSE2

#endif

#ifdef USE_SIMD
template<typename T> struct simd4 { typedef T simdType; };

template<> struct simd4<float> { typedef __m128 simdType; };
template<> struct simd4<int32_t> { typedef __m128i simdType; };

#if defined(USE_AVX2) || defined(USE_AVX)
	template<> struct simd4<double> { typedef __m256 simdType; };
	template<> struct simd4<int64_t> { typedef __m256i simdType; };
#endif

#endif