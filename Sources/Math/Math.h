#pragma once



#include "Tvec4.h"



// glm::core functions 
// https://glm.g-truc.net/0.9.2/api/a00239.html

// glm::simd functions
// https://glm.g-truc.net/0.9.2/api/a00293.html

// glm::fquat functions
// https://glm.g-truc.net/0.9.2/api/a00246.html

// glm::matrix functions
// https://glm.g-truc.net/0.9.2/api/a00245.html




template<typename T, typename T2>
T dot(const Tvec2<T>& a, const Tvec2<T2>& b)
{
	return a.x * b.x + a.y * b.y;
}

template<typename T, typename T2>
T dot(const Tvec3<T>& a, const Tvec3<T2>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T, typename T2>
T dot(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template<typename T, typename T2>
Tvec4<T> dot4(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return Tvec4<T>(dot(a, b));
}

template<typename T, typename T2>
Tvec3<T> cross(const Tvec3<T>& a, const Tvec3<T2>& b)
{
	return Tvec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template<typename T, typename T2>
Tvec4<T> cross(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return Tvec4<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0);
}

#ifdef USE_SIMD

/*Tvec4<float> dot4(const Tvec4<float>& a, const Tvec4<float>& b)
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return _mm_dp_ps(a.v128, b.v128, 0xFF);

#else //USE_SSE2
	__m128 v0 = _mm_mul_ps(a.v128, b.v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);
	return v0;

#endif
}

float dot(const Tvec4<float>& a, const Tvec4<float>& b)
{
	return _mm_cvtss_f32(dot4(a, b).v128);
}

Tvec4<float> cross(const Tvec4<float>& a, const Tvec4<float>& b)
{
	__m128 tmp0 = _mm_shuffle_ps(a.v128, a.v128, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 tmp1 = _mm_shuffle_ps(b.v128, b.v128, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 tmp2 = _mm_mul_ps(tmp0, b.v128);
	__m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
	__m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));

	Tvec4<float> result = _mm_sub_ps(tmp3, tmp4);
	result.w = 0;
	return result;
}
*/
#endif // USE_SIMD



