#pragma once
#include <iostream>

#include "Tvec4.h"



#ifdef USE_SIMD

// FLOAT

Tvec4<float> operator+(const Tvec4<float>& a, const Tvec4<float>& b)
{
	//std::cout << "Hello world" << std::endl;
	return _mm_add_ps(a.v128, b.v128);
}

Tvec4<float> operator-(const Tvec4<float>& a, const Tvec4<float>& b)
{
	return _mm_sub_ps(a.v128, b.v128);
}

Tvec4<float> operator*(const Tvec4<float>& a, const Tvec4<float>& b)
{
	return _mm_mul_ps(a.v128, b.v128);
}

void Tvec4<float>::operator*=(const float& scalar)
{
	v128 = _mm_mul_ps(v128, _mm_set_ps1(scalar));
}

void Tvec4<float>::operator/=(const float& scalar)
{
	v128 = _mm_div_ps(v128, _mm_set_ps1(scalar));
}

bool Tvec4<float>::operator==(const Tvec4<float>& b)
{
	return _mm_movemask_ps(_mm_cmpeq_ps(v128, b.v128)) == 0x0F;
}

bool Tvec4<float>::operator!=(const Tvec4<float>& b)
{
	return _mm_movemask_ps(_mm_cmpneq_ps(v128, b.v128)) == 0x0F;
}

Tvec4<float>& Tvec4<float>::operator=(const Tvec4<float>& b)
{
	v128 = b.v128;
	return *this;
}


float Tvec4<float>::getNorm() const
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return sqrt(_mm_cvtss_f32(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);

	return sqrt(_mm_cvtss_f32(v0));

#endif
}

Tvec4<float> Tvec4<float>::getNormal() const
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return _mm_mul_ps(v128, _mm_rsqrt_ps(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2

	// compute dot
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);

	// inverse sqrt and multiply
	v1 = _mm_mul_ps(v128, _mm_rsqrt_ps(v0));
	return v1;
#endif
}

void Tvec4<float>::normalize()
{
#if defined(USE_AVX2) || defined(USE_AVX)
	v128 = _mm_mul_ps(v128, _mm_rsqrt_ps(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2

	// compute dot
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);

	// inverse sqrt and multiply
	v128 = _mm_mul_ps(v128, _mm_rsqrt_ps(v0));
#endif
}

Tvec4<float> operator*(const float& scalar, const Tvec4<float>& a)
{
	return _mm_mul_ps(a.v128, Tvec4<float>(scalar).v128);
	//return _mm_mul_ps(a.v128, _mm_set_ps1(scalar));
}

Tvec4<float> operator*(const Tvec4<float>& a, const float& scalar)
{
	//return Tvec4<T>(scalar * a.x, scalar * a.y, scalar * a.z, scalar * a.w);
	return _mm_mul_ps(a.v128, Tvec4<float>(scalar).v128);
}

Tvec4<float> operator/(const Tvec4<float>& a, const float& scalar)
{
	return _mm_mul_ps(a.v128, Tvec4<float>(1.f / scalar).v128);
}


float dot(const Tvec4<float>& a, const Tvec4<float>& b)
{
	return _mm_cvtss_f32(_mm_dp_ps(a.v128, b.v128, 0xFF));
}



// INT

Tvec4<int32_t> operator+(const Tvec4<int32_t>& a, const Tvec4<int32_t>& b)
{
	return _mm_add_epi32(a.v128, b.v128);
}

Tvec4<int32_t> operator-(const Tvec4<int32_t>& a, const Tvec4<int32_t>& b)
{
	return _mm_sub_epi32(a.v128, b.v128);
}

Tvec4<int32_t> operator*(const Tvec4<int32_t>& a, const Tvec4<int32_t>& b)
{
	Tvec4<float> tmp = _mm_mul_ps(Tvec4<float>(a).v128, Tvec4<float>(b).v128);
	return tmp;
}

void Tvec4<int32_t>::operator*=(const int32_t& scalar)
{
	Tvec4<float> tmp = _mm_mul_ps(Tvec4<float>(*this).v128, Tvec4<float>((float)scalar).v128);
	*this = tmp;
}

void Tvec4<int32_t>::operator/=(const int32_t& scalar)
{
	Tvec4<float> tmp = _mm_div_ps(Tvec4<float>(*this).v128, Tvec4<float>((float)scalar).v128);
	*this = tmp;
}

bool Tvec4<int32_t>::operator==(const Tvec4<int32_t>& b)
{
	return _mm_movemask_epi8(_mm_cmpeq_epi32(v128, b.v128)) == 0xFFFF;
}

bool Tvec4<int32_t>::operator!=(const Tvec4<int32_t>& b)
{
	return _mm_movemask_epi8(_mm_cmpeq_epi32(v128, b.v128)) != 0xFFFF;
}

template<>
Tvec4<int32_t>& Tvec4<int32_t>::operator=(const Tvec4<int32_t>& b)
{
	v128 = b.v128;
	return *this;
}


/*float Tvec4<int32_t>::getNorm() const
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return sqrt(_mm_cvtss_f32(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);

	return sqrt(_mm_cvtss_f32(v0));

#endif
}

Tvec4<float> Tvec4<int32_t>::getNormal() const
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return _mm_mul_ps(v128, _mm_rsqrt_ps(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2

	// compute dot
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);

	// inverse sqrt and multiply
	v1 = _mm_mul_ps(v128, _mm_rsqrt_ps(v0));
	return v1;
#endif
}

void Tvec4<int32_t>::normalize()
{
#if defined(USE_AVX2) || defined(USE_AVX)
	v128 = _mm_mul_ps(v128, _mm_rsqrt_ps(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2

	// compute dot
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);

	// inverse sqrt and multiply
	v128 = _mm_mul_ps(v128, _mm_rsqrt_ps(v0));
#endif
}*/

Tvec4<int32_t> operator*(const int32_t& scalar, const Tvec4<int32_t>& a)
{
	Tvec4<float> tmp = _mm_mul_ps(Tvec4<float>(a).v128, _mm_set_ps1((float)scalar));
	return tmp;
}

Tvec4<int32_t> operator*(const Tvec4<int32_t>& a, const int32_t& scalar)
{
	Tvec4<float> tmp = _mm_mul_ps(Tvec4<float>(a).v128, _mm_set_ps1((float)scalar));
	return tmp;
}

Tvec4<int32_t> operator/(const Tvec4<int32_t>& a, const int32_t& scalar)
{
	Tvec4<float> tmp = _mm_div_ps(Tvec4<float>(a).v128, _mm_set_ps1((float)scalar));
	return tmp;
}


#endif








// AS USUAL

template<typename T, typename T2>
Tvec4<T> operator+(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return Tvec4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template<typename T, typename T2>
Tvec4<T> operator-(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return Tvec4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template<typename T, typename T2>
Tvec4<T> operator*(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return Tvec4<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

template<typename T>
void Tvec4<T>::operator*=(const T& scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
}

template<typename T>
void Tvec4<T>::operator/=(const T& scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
}

template<typename T>
bool Tvec4<T>::operator==(const Tvec4& b)
{
	return x == b.x && y == b.y && z == b.z && w == b.w;
}

template<typename T>
bool Tvec4<T>::operator!=(const Tvec4& b)
{
	return x != b.x && y != b.y && z != b.z && w != b.w;
}

template<typename T>
Tvec4<T>& Tvec4<T>::operator=(const Tvec4& b)
{
	x = b.x;
	y = b.y;
	z = b.z;
	w = b.w;
	return *this;
}

template<typename T>
T Tvec4<T>::getNorm() const
{
	return sqrt(x * x + y * y + z * z + w * w);
}

template<typename T>
Tvec4<T> Tvec4<T>::getNormal() const
{
	T invnorm = T(1) / sqrt(x * x + y * y + z * z + w * w);
	return Tvec4<T>(x * invnorm, y * invnorm, z * invnorm, w * invnorm);
}

template<typename T>
void Tvec4<T>::normalize()
{
	T invnorm = T(1) / sqrt(x * x + y * y + z * z + w * w);
	x *= invnorm;
	y *= invnorm;
	z *= invnorm;
	w *= invnorm;
}

template<typename T, typename T2>
Tvec4<T> operator*(const T2& scalar, const Tvec4<T>& a)
{
	return Tvec4<T>(scalar * a.x, scalar * a.y, scalar * a.z, scalar * a.w);
}

template<typename T, typename T2>
Tvec4<T> operator*(const Tvec4<T>& a, const T2& scalar)
{
	return Tvec4<T>(scalar * a.x, scalar * a.y, scalar * a.z, scalar * a.w);
}

template<typename T, typename T2>
Tvec4<T> operator/(const Tvec4<T>& a, const T2& scalar)
{
	return Tvec4<T>(a.x / scalar, a.y / scalar, a.z / scalar, a.w / scalar);
}


template<typename T>
float dot(const Tvec4<T>& a, const Tvec4<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}