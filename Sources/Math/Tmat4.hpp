#pragma once

#include "Tmat4.h"


// operators
template<typename T>
Tmat4<T>& Tmat4<T>::operator=(const Tmat4& b)
{
	for (int i = 0; i < 4; ++i)
		c[i] = b.c[i];
	return *this;
}

template<typename T>
void Tmat4<T>::operator*=(const T& scalar)
{
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		m[i][j] *= scalar;
	c[0] *= scalar;
	c[1] *= scalar;
	c[2] *= scalar;
	c[3] *= scalar;
}

template<typename T>
void Tmat4<T>::operator/=(const T& scalar)
{
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		m[i][j] /= scalar;
	T denom = T(1) / scalar;
	c[0] *= denom;
	c[1] *= denom;
	c[2] *= denom;
	c[3] *= denom;
}

template<typename T>
void Tmat4<T>::operator+=(const Tmat4& b)
{
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		m[i][j] += b[i][j];
	c[0] += b[0];
	c[1] += b[1];
	c[2] += b[2];
	c[3] += b[3];
}

template<typename T, typename T2>
Tmat4<T> operator+(const Tmat4<T>& a, const Tmat4<T2>& b)
{
	Tmat4<T> result = Tmat4<T>(0);
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][j] + b[i][j];
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
	result[2] = a[2] + b[2];
	result[3] = a[3] + b[3];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator-(const Tmat4<T>& a, const Tmat4<T2>& b)
{
	Tmat4<T> result = Tmat4<T>(0);
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][j] - b[i][j];
	result[0] = a[0] - b[0];
	result[1] = a[1] - b[1];
	result[2] = a[2] - b[2];
	result[3] = a[3] - b[3];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator*(const Tmat4<T>& a, const T2& scalar)
{
	Tmat4<T> result;// = Tmat4<T>(0);
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][j] * scalar;

	result[0] = scalar * a[0];
	result[1] = scalar * a[1];
	result[2] = scalar * a[2];
	result[3] = scalar * a[3];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator/(const Tmat4<T>& a, const T2& scalar)
{
	Tmat4<T> result;// = Tmat4<T>(0);
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][j] / scalar;
	T2 denom = T2(1) / scalar;
	result[0] = denom * a[0];
	result[1] = denom * a[1];
	result[2] = denom * a[2];
	result[3] = denom * a[3];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator*(const T2& scalar, const Tmat4<T>& a)
{
	Tmat4<T> result;// = Tmat4<T>(0);
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][j] * scalar;
	result[0] = scalar * a[0];
	result[1] = scalar * a[1];
	result[2] = scalar * a[2];
	result[3] = scalar * a[3];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator/(const T2& scalar, const Tmat4<T>& a)
{
	Tmat4<T> result;// = Tmat4<T>(0);
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][j] / scalar;
	T2 denom = T2(1) / scalar;
	result[0] = denom * a[0];
	result[1] = denom * a[1];
	result[2] = denom * a[2];
	result[3] = denom * a[3];
	return result;
}


//math
template<typename T>
T Tmat4<T>::det() const
{
	T a = m[0][0];
	T b = m[0][1];
	T c = m[0][2];
	T d = m[0][3];
	T e = m[1][0];
	T f = m[1][1];
	T g = m[1][2];
	T h = m[1][3];
	T i = m[2][0];
	T j = m[2][1];
	T k = m[2][2];
	T l = m[2][3];
	T q = m[3][0];
	T n = m[3][1];
	T o = m[3][2];
	T p = m[3][3];

	return a * f * k * p - a * f * l * o - a * g * j * p + a * g * l * n +
		a * h * j * o - a * h * k * n - b * e * k * p + b * e * l * o +
		b * g * i * p - b * g * l * q - b * h * i * o + b * h * k * q +
		c * e * j * p - c * e * l * n - c * f * i * p + c * f * l * q +
		c * h * i * n - c * h * j * q - d * e * j * o + d * e * k * n +
		d * f * i * o - d * f * k * q - d * g * i * n + d * g * j * q;
}

template<typename T>
Tquat<T> Tmat4<T>::extractRotation() const
{
	T fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
	T fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
	T fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
	T fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

	int biggestIndex = 0;
	T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	T biggestVal = sqrt(fourBiggestSquaredMinus1 + T(1)) * T(0.5);
	T mult = static_cast<T>(0.25) / biggestVal;

	Tquat<T> Result;
	switch (biggestIndex)
	{
	case 0:
		Result.w = biggestVal;
		Result.x = (m[1][2] - m[2][1]) * mult;
		Result.y = (m[2][0] - m[0][2]) * mult;
		Result.z = (m[0][1] - m[1][0]) * mult;
		break;
	case 1:
		Result.w = (m[1][2] - m[2][1]) * mult;
		Result.x = biggestVal;
		Result.y = (m[0][1] + m[1][0]) * mult;
		Result.z = (m[2][0] + m[0][2]) * mult;
		break;
	case 2:
		Result.w = (m[2][0] - m[0][2]) * mult;
		Result.x = (m[0][1] + m[1][0]) * mult;
		Result.y = biggestVal;
		Result.z = (m[1][2] + m[2][1]) * mult;
		break;
	case 3:
		Result.w = (m[0][1] - m[1][0]) * mult;
		Result.x = (m[2][0] + m[0][2]) * mult;
		Result.y = (m[1][2] + m[2][1]) * mult;
		Result.z = biggestVal;
		break;

	default:					// Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
		GF_ASSERT(false);
		break;
	}
	return Result;

	// remove potential scale
	/*Tvec4<T> c0 = c[0].getNormal();
	Tvec4<T> c1 = c[1].getNormal();
	Tvec4<T> c2 = c[2].getNormal();

	Tquat<T> q;
	T tr = c0[0] + c1[1] + c2[2];
	if (tr > 0)
	{
		T s = std::sqrt(tr + 1) * 2;
		q.w = T(0.25) * s;
		q.x = (c2[1] - c1[2]) / s;
		q.y = (c0[2] - c2[0]) / s;
		q.z = (c1[0] - c0[1]) / s;
	}
	else if (c0[0] > c1[1] && c0[0] > c2[2])
	{
		T s = std::sqrt(1 + c0[0] - c1[1] - c2[2]) * 2;
		q.w = (c2[1] - c1[2]) / s;
		q.x = T(0.25) * s;
		q.y = (c0[1] + c1[0]) / s;
		q.z = (c0[2] + c2[0]) / s;
	}
	else if (c1[1] > c2[2])
	{
		T s = std::sqrt(1 + c1[1] - c0[0] - c2[2]) * 2;
		q.w = (c0[2] - c2[0]) / s;
		q.x = (c0[1] + c1[0]) / s;
		q.y = T(0.25) * s;
		q.z = (c2[1] + c1[2]) / s;
	}
	else
	{
		T s = std::sqrt(1 + c2[2] - c0[0] - c1[1]) * 2;
		q.w = (c1[0] - c0[1]) / s;
		q.x = (c0[2] + c2[0]) / s;
		q.y = (c2[1] + c1[2]) / s;
		q.z = T(0.25) * s;
	}
	return q;*/
}


// constants
template<typename T> const Tmat4<T> Tmat4<T>::identity = Tmat4<T>();



// math function
template<typename T, typename T2>
Tmat4<T> operator*(const Tmat4<T>& a, const Tmat4<T2>& b)
{
	//Tmat4<T> result;
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		result[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];

	Tmat4<T> result;
	result[0] = a[0] * b[0][0] + a[1] * b[0][1] + a[2] * b[0][2] + a[3] * b[0][3];
	result[1] = a[0] * b[1][0] + a[1] * b[1][1] + a[2] * b[1][2] + a[3] * b[1][3];
	result[2] = a[0] * b[2][0] + a[1] * b[2][1] + a[2] * b[2][2] + a[3] * b[2][3];
	result[3] = a[0] * b[3][0] + a[1] * b[3][1] + a[2] * b[3][2] + a[3] * b[3][3];
	return result;
}

template<typename T, typename T2>
Tvec4<T> operator*(const Tmat4<T>& a, const Tvec4<T2>& v)
{
	Tvec4<T> result;
	//for (int i = 0; i < 4; ++i)
	//	result[i] = a[i][0] * v[0] + a[i][1] * v[1] + a[i][2] * v[2] + a[i][3] * v[3];
	result = a[0] * v[0] + a[1] * v[1] + a[2] * v[2] + a[3] * v[3];
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::translate(const Tmat4<T>& m, const Tvec4<T>& t)
{
	Tmat4<T> result = m;
	result[3] = m[0] * t[0] + m[1] * t[1] + m[2] * t[2] + m[3];
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::scale(const Tmat4<T>& m, const Tvec4<T>& s)
{
	Tmat4<T> result = m;
	result[0] *= s.x;
	result[1] *= s.y;
	result[2] *= s.z;
	result[3] *= s.w;
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::rotate(const Tmat4<T>& m, const Tquat<T> q)
{
	return Tmat4<T>(q) * m;
}

template<typename T>
Tmat4<T> Tmat4<T>::rotate(const Tmat4<T>& m, const Tvec3<T> euler)
{
	return rotate(m, Tquat<T>(euler));

	/*T cx = cos(euler.x); T sx = sin(euler.x);
	T cy = cos(euler.y); T sy = sin(euler.y);
	T cz = cos(euler.z); T sz = sin(euler.z);
	Tmat4<T> result;

	result[0][0] = cy * cz;
	result[0][1] = cy * sz;
	result[0][2] = -sy;
	result[0][3] = 0;

	result[1][0] = sx * sy * cz - cx * sz;
	result[1][1] = sx * sy * sz + cx * cz;
	result[1][2] = sx * cy;
	result[1][3] = 0;

	result[1][0] = cx * sy * cz + sx * sz;
	result[1][1] = cx * sy * sz - sx * cz;
	result[1][2] = cx * cy;
	result[1][3] = 0;

	result[1][0] = 0;
	result[1][1] = 0;
	result[1][2] = 0;
	result[1][3] = 1;

	return result * m;*/
}

template<typename T>
Tmat4<T> Tmat4<T>::eulerAngleZX(T z, T x)
{
	T cx = cos(x); T sx = sin(x);
	T cz = cos(z); T sz = sin(z);
	Tmat4<T> result;

	result[0][0] = cz;
	result[0][1] = -sz;
	result[0][2] = 0;
	result[0][3] = 0;

	result[1][0] = cx * sz;
	result[1][1] = cx * cz;
	result[1][2] = -sx;
	result[1][3] = 0;

	result[1][0] = sx * sz;
	result[1][1] = sx * cz;
	result[1][2] = cx;
	result[1][3] = 0;

	result[1][0] = 0;
	result[1][1] = 0;
	result[1][2] = 0;
	result[1][3] = 1;
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::fromTo(const Tvec4<T>& from, const Tvec4<T>& to)
{
	Tquat<T> q = Tquat<T>(from, to);
	return Tmat4<T>(q);
}

template<typename T>
Tmat4<T> Tmat4<T>::lookAt(const Tvec4<T>& forward, Tvec4<T> up)
{
	Tmat4<T> result;
	result[0] = forward;
	result[2] = Tvec4<T>::cross(forward, up);
	result[1] = Tvec4<T>::cross(result[2], forward);
	result[3] = Tvec4<T>(0, 0, 0, 1);
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::ortho(const T left, const T right, const T bottom, const T top, const T zNear, const T zFar)
{
	Tmat4<T> result;
	result[0][0] = static_cast<T>(2) / (right - left);
	result[1][1] = static_cast<T>(2) / (top - bottom);
	result[2][2] = -static_cast<T>(2) / (zFar - zNear);
	result[3][0] = -(right + left) / (right - left);
	result[3][1] = -(top + bottom) / (top - bottom);
	result[3][2] = -(zFar + zNear) / (zFar - zNear);
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::TRS(const Tvec4<T>& t, const Tquat<T> q, const Tvec4<T>& s)
{
	//Tmat4<T> result = Tmat4<T>::translate(Tmat4<T>::identity, t);
	//result = Tmat4<T>::rotate(result, q);
	//return Tmat4<T>::scale(result, s);

	Tmat4<T> result(q);
	result[0] *= s[0];
	result[1] *= s[1];
	result[2] *= s[2];
	result[3] = t;
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::inverse(const Tmat4<T>& m)
{
	//T det = m.det();

	T A2323 = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
	T A1323 = m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1];
	T A1223 = m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1];
	T A0323 = m.m[2][0] * m.m[3][3] - m.m[2][3] * m.m[3][0];
	T A0223 = m.m[2][0] * m.m[3][2] - m.m[2][2] * m.m[3][0];
	T A0123 = m.m[2][0] * m.m[3][1] - m.m[2][1] * m.m[3][0];
	T A2313 = m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2];
	T A1313 = m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1];
	T A1213 = m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1];
	T A2312 = m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2];
	T A1312 = m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1];
	T A1212 = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
	T A0313 = m.m[1][0] * m.m[3][3] - m.m[1][3] * m.m[3][0];
	T A0213 = m.m[1][0] * m.m[3][2] - m.m[1][2] * m.m[3][0];
	T A0312 = m.m[1][0] * m.m[2][3] - m.m[1][3] * m.m[2][0];
	T A0212 = m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0];
	T A0113 = m.m[1][0] * m.m[3][1] - m.m[1][1] * m.m[3][0];
	T A0112 = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];

	T det = m.m[0][0] * (m.m[1][1] * A2323 - m.m[1][2] * A1323 + m.m[1][3] * A1223)
		- m.m[0][1] * (m.m[1][0] * A2323 - m.m[1][2] * A0323 + m.m[1][3] * A0223)
		+ m.m[0][2] * (m.m[1][0] * A1323 - m.m[1][1] * A0323 + m.m[1][3] * A0123)
		- m.m[0][3] * (m.m[1][0] * A1223 - m.m[1][1] * A0223 + m.m[1][2] * A0123);
	det = T(1) / det;

	return Tmat4<T>(det * (m.m[1][1] * A2323 - m.m[1][2] * A1323 + m.m[1][3] * A1223),
	   det * -(m.m[0][1] * A2323 - m.m[0][2] * A1323 + m.m[0][3] * A1223),
	   det *  (m.m[0][1] * A2313 - m.m[0][2] * A1313 + m.m[0][3] * A1213),
	   det * -(m.m[0][1] * A2312 - m.m[0][2] * A1312 + m.m[0][3] * A1212),
	   det * -(m.m[1][0] * A2323 - m.m[1][2] * A0323 + m.m[1][3] * A0223),
	   det *  (m.m[0][0] * A2323 - m.m[0][2] * A0323 + m.m[0][3] * A0223),
	   det * -(m.m[0][0] * A2313 - m.m[0][2] * A0313 + m.m[0][3] * A0213),
	   det *  (m.m[0][0] * A2312 - m.m[0][2] * A0312 + m.m[0][3] * A0212),
	   det *  (m.m[1][0] * A1323 - m.m[1][1] * A0323 + m.m[1][3] * A0123),
	   det * -(m.m[0][0] * A1323 - m.m[0][1] * A0323 + m.m[0][3] * A0123),
	   det *  (m.m[0][0] * A1313 - m.m[0][1] * A0313 + m.m[0][3] * A0113),
	   det * -(m.m[0][0] * A1312 - m.m[0][1] * A0312 + m.m[0][3] * A0112),
	   det * -(m.m[1][0] * A1223 - m.m[1][1] * A0223 + m.m[1][2] * A0123),
	   det *  (m.m[0][0] * A1223 - m.m[0][1] * A0223 + m.m[0][2] * A0123),
	   det * -(m.m[0][0] * A1213 - m.m[0][1] * A0213 + m.m[0][2] * A0113),
	   det *  (m.m[0][0] * A1212 - m.m[0][1] * A0212 + m.m[0][2] * A0112)
	);
}

template<typename T>
Tmat4<T> Tmat4<T>::transpose(const Tmat4<T>& m)
{
	Tmat4<T> result;
	for (uint8_t i = 0; i < 4; i++)
		for (uint8_t j = 0; j < 4; j++)
			result[i][j] = m[j][i];
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::perspective(T fovy, T aspect, T zNear, T zFar)
{
	const T tanHalfFovy = tan(fovy * T(0.5));
	const T denom = (T)1 / (zFar - zNear);
	Tmat4<T> result(0);
	result[0][0] = (T)1 / (aspect * tanHalfFovy);
	result[1][1] = (T)1 / (tanHalfFovy);
	result[2][2] = -(zFar + zNear) * denom;
	result[2][3] = -1;
	result[3][2] = -(2 * zFar * zNear) * denom;
	return result;
}