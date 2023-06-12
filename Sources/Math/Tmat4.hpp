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
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			m[i][j] *= scalar;
}

template<typename T>
void Tmat4<T>::operator/=(const T& scalar)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			m[i][j] /= scalar;
}

template<typename T>
void Tmat4<T>::operator+=(const Tmat4& b)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			m[i][j] += b[i][j];
}

template<typename T, typename T2>
Tmat4<T> operator+(const Tmat4<T>& a, const Tmat4<T2>& b)
{
	Tmat4<T> result = Tmat4<T>(0);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][j] + b[i][j];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator-(const Tmat4<T>& a, const Tmat4<T2>& b)
{
	Tmat4<T> result = Tmat4<T>(0);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][j] - b[i][j];
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator*(const Tmat4<T>& a, const T2& scalar)
{
	Tmat4<T> result = Tmat4<T>(0);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][j] * scalar;
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator/(const Tmat4<T>& a, const T2& scalar)
{
	Tmat4<T> result = Tmat4<T>(0);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][j] / scalar;
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator*(const T2& scalar, const Tmat4<T>& a)
{
	Tmat4<T> result = Tmat4<T>(0);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][j] * scalar;
	return result;
}

template<typename T, typename T2>
Tmat4<T> operator/(const T2& scalar, const Tmat4<T>& a)
{
	Tmat4<T> result = Tmat4<T>(0);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][j] / scalar;
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
	// remove potential scale
	Tvec4<T> c0 = c[0].getNormal();
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
	return q;
}


// constants
template<typename T> const Tmat4<T> Tmat4<T>::identity = Tmat4<T>();



// math function
template<typename T, typename T2>
Tmat4<T> operator*(const Tmat4<T>& a, const Tmat4<T2>& b)
{
	Tmat4<T> result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
	return result;
}

template<typename T, typename T2>
Tvec4<T> operator*(const Tmat4<T>& a, const Tvec4<T2>& v)
{
	Tvec4<T> result;
	for (int i = 0; i < 4; ++i)
		result[i] = a[i][0] * v[0] + a[i][1] * v[1] + a[i][2] * v[2] + a[i][3] * v[3];
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::translate(const Tmat4<T>& m, const Tvec4<T>& t)
{
	Tmat4<T> result = m;
	result.c[3] += t;
	return result;
}

template<typename T>
Tmat4<T> Tmat4<T>::scale(const Tmat4<T>& m, const Tvec4<T>& s)
{
	Tmat4<T> result = m;
	for (int i = 0; i < 4; ++i)
		result.c[i] *= s[i];
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
	T cx = cos(euler.x); T sx = sin(euler.x);
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

	return result * m;
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
	result.c[0] = forward;
	result.c[2] = Tvec4<T>::cross(forward, up);
	result.c[1] = Tvec4<T>::cross(result.c[2], forward);
	result.c[3] = Tvec4<T>(0, 0, 0, 1);
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
	Tmat4<T> result = Tmat4<T>::translate(Tmat4<T>::identity, t);
	result = Tmat4<T>::rotate(result, q);
	return Tmat4<T>::scale(result, s);
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
	T const tanHalfFovy = tan(fovy * T(0.5));
	Tmat4<T> result(0);
	result[0][0] = 1 / (aspect * tanHalfFovy);
	result[1][1] = 1 / (tanHalfFovy);
	result[2][2] = -(zFar + zNear) / (zFar - zNear);
	result[2][3] = -1;
	result[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
	return result;
}