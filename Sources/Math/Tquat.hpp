#pragma once


#include "Tquat.h"

// constructors
template<typename T>
Tquat<T>::Tquat(const Tvec4<T>& from, const Tvec4<T>& to)
{
	Tvec4<T> axis = Tvec4<T>::cross(from, to);
	x = axis.x;
	y = axis.y;
	z = axis.z;
	w = std::sqrt(from.getNorm2() * to.getNorm2()) + Tvec4<T>::dot(from, to);
	normalize();
}

template<typename T>
Tquat<T>::Tquat(const T& angle, const Tvec4<T>& axis)
{
	Tvec4<T> v = axis.getNormal();
	T ca = cos(T(0.5) * angle);
	T sa = sin(T(0.5) * angle);
	x = v.x * sa;
	y = v.y * sa;
	z = v.z * sa;
	w = ca;
}

// operator
template<typename T>
Tvec3<T> Tquat<T>::xyz() const
{
	return Tvec3<T>(x, y, z);
}

template<typename T>
Tvec4<T> Tquat<T>::xyzw() const
{
	return Tvec4<T>(x, y, z, w);
}

template<typename T>
Tquat<T> Tquat<T>::operator-() const
{
	Tquat<T> q = *this;
	q.x = -x;
	q.y = -y;
	q.z = -z;
	return q;
}

template<typename T>
void Tquat<T>::operator*=(const T& scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
}

template<typename T>
void Tquat<T>::operator/=(const T& scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
}


template<typename T>
bool Tquat<T>::operator==(const Tquat& b) const
{
	return (x == b.x && y == b.y && z == b.z && w == b.w);
}

template<typename T>
bool Tquat<T>::operator!=(const Tquat& b) const
{
	return (x != b.x || y != b.y || z != b.z || w != b.w);
}

template<typename T>
Tquat<T>& Tquat<T>::operator=(const Tquat& b)
{
	x = b.x;
	y = b.y;
	z = b.z;
	w = b.w;
	return *this;
}

template<typename T>
void Tquat<T>::normalize()
{
	T invNorm = std::sqrt(x * x + y * y + z * z + w * w);
	x *= invNorm;
	y *= invNorm;
	z *= invNorm;
	w *= invNorm;
}

//constants
template<typename T> const Tquat<T> Tquat<T>::identity = Tquat<T>(0, 0, 0, 1);


// math function
template<typename T>
Tvec4<T> operator*(const Tquat<T>& q, const Tvec4<T>& v)
{
	Tquat<T> vq = Tquat<T>(v.x, v.y, v.z, 0);
	Tquat<T> r = q * vq * (-q);
	return Tvec4<T>(r.x, r.y, r.z, v.w);
}

template<typename T>
Tquat<T> operator*(const Tquat<T>& a, const Tquat<T>& b)
{
	Tquat<T> result;
	result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	return result;
}

template<typename T, typename T2>
Tquat<T> operator*(const Tquat<T>& a, const T2& scalar)
{
	return Tquat<T>(scalar * a.x, scalar * a.y, scalar * a.z, scalar * a.w);
}

template<typename T, typename T2>
Tquat<T> operator*(const T2& scalar, const Tquat<T>& a)
{
	return Tquat<T>(scalar * a.x, scalar * a.y, scalar * a.z, scalar * a.w);
}

template<typename T, typename T2>
Tquat<T> operator/(const Tquat<T>& a, const T2& scalar)
{
	return Tquat<T>(a.x / scalar, a.y / scalar, a.z / scalar, a.w / scalar);
}

template<typename T, typename T2>
Tquat<T> operator/(const T2& scalar, const Tquat<T>& a)
{
	return Tquat<T>(a.x / scalar, a.y / scalar, a.z / scalar, a.w / scalar);
}

template<typename T, typename T2>
Tquat<T> operator+(const Tquat<T>& a, const Tquat<T2>& b)
{
	return Tquat<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template<typename T>
Tquat<T> Tquat<T>::slerp(const Tquat<T>& a, const Tquat<T>& b, const T& t)
{
	Tquat<T> z = b;
	T cosTheta = Tvec4<T>::dot(a.xyzw(), b.xyzw());
	if (cosTheta < 0)
	{
		z = -b;
		cosTheta = -cosTheta;
	}

	if (cosTheta > T(1) - EPSILON)
		return Tquat<T>(lerp(a.x, z.x, t), lerp(a.y, z.y, t), lerp(a.z, z.z, t), lerp(a.w, z.w, t));
	else
	{
		T angle = acos(cosTheta);
		return (sin((T(1) - t) * angle) * a + sin(t * angle) * z) / sin(angle);
	}
}

template<typename T>
Tquat<T> Tquat<T>::lookAt(const Tvec4<T>& forward, Tvec4<T> up)
{
	Tvec4<T> right = Tvec4<T>::cross(forward, up);
	up = Tvec4<T>::cross(right, forward);
	Tvec4<T> c0 = forward;
	Tvec4<T> c1 = up;
	Tvec4<T> c2 = right;

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