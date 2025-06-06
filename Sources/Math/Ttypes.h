#pragma once


#include <inttypes.h>


template <typename T> class Tvec2;
template <typename T> class Tvec3;
template <typename T> class Tvec4;
template <typename T> class Tmat4;
template <typename T> class Tquat;

typedef Tvec2<float>		vec2f;
typedef Tvec2<double>		vec2d;
typedef Tvec2<int32_t>		vec2i;
//typedef Tvec2<uint32_t>		vec2ui;
//typedef Tvec2<bool>			vec2b;

typedef Tvec3<float>		vec3f;
typedef Tvec3<double>		vec3d;
typedef Tvec3<int32_t>		vec3i;
//typedef Tvec3<uint32_t>		vec3ui;
//typedef Tvec3<bool>			vec3b;

typedef Tvec4<float>		vec4f;
typedef Tvec4<double>		vec4d;
typedef Tvec4<int32_t>		vec4i;
typedef Tvec4<uint32_t>		vec4ui;
typedef Tvec4<bool>			vec4b;

typedef Tmat4<float>		mat4f;
typedef Tmat4<double>		mat4d;

typedef Tquat<float>		quatf;
typedef Tquat<double>		quatd;


#define EPSILON 1E-6
#define PI 3.14159265359
#define DEG2RAD (PI / 180.0)
#define RAD2DEG (180.0 / PI)

template<typename T>
T lerp(const T& a, const T& b, const T& t) {
	return a + t * (b - a);
}

template<typename T>
T clamp(const T& a, const T& min, const T& max) {
	return a < min ? min : (a > max ? max : a);
}

template<typename T>
Tvec2<T> clamp(const Tvec2<T>& a, const Tvec2<T>& min, const Tvec2<T>& max) 
{
	return Tvec2<T>(clamp(a.x, min.x, max.x), clamp(a.y, min.y, max.y));
}

template<typename T>
Tvec3<T> clamp(const Tvec3<T>& a, const Tvec3<T>& min, const Tvec3<T>& max) 
{
	return Tvec3<T>(clamp(a.x, min.x, max.x), clamp(a.y, min.y, max.y), clamp(a.z, min.z, max.z));
}

template<typename T>
Tvec4<T> clamp(const Tvec4<T>& a, const Tvec4<T>& min, const Tvec4<T>& max) 
{
	return Tvec4<T>(clamp(a.x, min.x, max.x), clamp(a.y, min.y, max.y), clamp(a.z, min.z, max.z), clamp(a.w, min.w, max.w));
}


#ifdef _DEBUG
	#undef USE_SIMD

#elif defined(__AVX2__) || defined(__AVX__)
	#include <xmmintrin.h>
	#include <emmintrin.h>
	#include <immintrin.h>
	#include <intrin.h>

	//#define USE_SIMD //(GLM_ARCH_AVX | GLM_ARCH_SSE4 | GLM_ARCH_SSE3 | GLM_ARCH_SSE2)
	//#define USE_AVX

#if defined(USE_AVX2)
	#define USE_AVX
	#define USE_SIMD
#elif defined(USE_AVX)
	#define USE_SIMD
#endif

#elif (_M_IX86_FP == 2) || defined(_M_X64) || defined (_M_AMD64)
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

#include <cmath>
#include <Utiles/Assert.hpp>

