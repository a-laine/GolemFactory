#pragma once
#include <iostream>

#include "Tvec4.h"



#ifdef USE_SIMD

// FLOAT
/*Tvec4<float> Tvec4<float>::operator-() const
{
	return _mm_xor_ps(v128, Tvec4<float>(-0.0).v128);
	//return _mm_sub_ps(Tvec4<float>::zero.v128, v128);
}

void Tvec4<float>::operator*=(const float& scalar)
{
	v128 = _mm_mul_ps(v128, Tvec4<float>(scalar).v128);
}

void Tvec4<float>::operator/=(const float& scalar)
{
	v128 = _mm_div_ps(v128, Tvec4<float>(scalar).v128);
}

void Tvec4<float>::operator*=(const Tvec4<float>& b)
{
	v128 = _mm_mul_ps(v128, b.v128);
}

void Tvec4<float>::operator/=(const Tvec4<float>& b)
{
	v128 = _mm_div_ps(v128, b.v128);
}

void Tvec4<float>::operator+=(const Tvec4<float>& b)
{
	v128 = _mm_add_ps(v128, b.v128);
}

void Tvec4<float>::operator-=(const Tvec4<float>& b)
{
	v128 = _mm_sub_ps(v128, b.v128);
}

bool Tvec4<float>::operator==(const Tvec4<float>& b) const
{
	return _mm_movemask_ps(_mm_cmpeq_ps(v128, b.v128)) == 0x0F;
}

bool Tvec4<float>::operator!=(const Tvec4<float>& b) const
{
	return _mm_movemask_ps(_mm_cmpneq_ps(v128, b.v128)) == 0x0F;
}

Tvec4<float>& Tvec4<float>::operator=(const Tvec4<float>& b)
{
	v128 = b.v128;
	return *this;
}*/

/*float Tvec4<float>::getNorm() const
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return _mm_cvtss_f32(_mm_sqrt_ps(_mm_dp_ps(v128, v128, 0xFF)));

#else //USE_SSE2
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);
	return _mm_cvtss_f32(_mm_sqrt_ps(v0));

#endif
}

Tvec4<float> Tvec4<float>::getNorm4() const
{
#if defined(USE_AVX2) || defined(USE_AVX)
	return _mm_sqrt_ps(_mm_dp_ps(v128, v128, 0xFF));

#else //USE_SSE2
	__m128 v0 = _mm_mul_ps(v128, v128);
	__m128 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
	v0 = _mm_add_ps(v0, v1);
	v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
	v0 = _mm_add_ps(v0, v1);
	return _mm_sqrt_ps(v0);

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

Tvec4<float> operator/(const Tvec4<float>& a, const Tvec4<float>& b)
{
	return _mm_div_ps(a.v128, b.v128);
}

Tvec4<float> operator*(const float& scalar, const Tvec4<float>& a)
{
	return _mm_mul_ps(a.v128, Tvec4<float>(scalar).v128);
}

Tvec4<float> operator*(const Tvec4<float>& a, const float& scalar)
{
	return _mm_mul_ps(a.v128, Tvec4<float>(scalar).v128);
}

Tvec4<float> operator/(const Tvec4<float>& a, const float& scalar)
{
	//return _mm_mul_ps(a.v128, Tvec4<float>(1.f / scalar).v128);
	return _mm_div_ps(a.v128, Tvec4<float>(scalar).v128);
}
*/
#endif








// AS USUAL
template<typename T>
Tvec4<T> Tvec4<T>::operator-() const
{
	return Tvec4<T>(-x, -y, -z, -w);
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
void Tvec4<T>::operator*=(const Tvec4& b)
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	w *= b.w;
}

template<typename T>
void Tvec4<T>::operator/=(const Tvec4& b)
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	w /= b.w;
}

template<typename T>
void Tvec4<T>::operator+=(const Tvec4& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
	w += b.w;
}

template<typename T>
void Tvec4<T>::operator-=(const Tvec4& b)
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	w -= b.w;
}

template<typename T>
bool Tvec4<T>::operator==(const Tvec4& b) const
{
	return x == b.x && y == b.y && z == b.z && w == b.w;
}

template<typename T>
bool Tvec4<T>::operator!=(const Tvec4& b) const
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
Tvec4<T> Tvec4<T>::getNorm4() const
{
	return Tvec4<T>(sqrt(x * x + y * y + z * z + w * w));
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

template<typename T, typename T2>
Tvec4<T> operator/(const Tvec4<T>& a, const Tvec4<T2>& b)
{
	return Tvec4<T>(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
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


template<typename T> const Tvec4<T> Tvec4<T>::zero = Tvec4<T>(0);
template<typename T> const Tvec4<T> Tvec4<T>::one = Tvec4<T>(1);
