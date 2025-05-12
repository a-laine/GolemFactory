#pragma once


#include "Tquat.h"
#include "Tmat3.h"
#include "Tmat4.h"

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

template<typename T>
Tquat<T>::Tquat(const T& angle, const Tvec3<T>& axis)
{
	Tvec3<T> v = axis.getNormal();
	T ca = cos(T(0.5) * angle);
	T sa = sin(T(0.5) * angle);
	x = v.x * sa;
	y = v.y * sa;
	z = v.z * sa;
	w = ca;
}

template<typename T>
Tquat<T>::Tquat(const Tvec3<T>& eulers)
{
	Tvec3<T> c = Tvec3<T>(cos(eulers.x * T(0.5)), cos(eulers.y * T(0.5)), cos(eulers.z * T(0.5)));
	Tvec3<T> s = Tvec3<T>(sin(eulers.x * T(0.5)), sin(eulers.y * T(0.5)), sin(eulers.z * T(0.5)));

	w = c.x * c.y * c.z + s.x * s.y * s.z;
	x = s.x * c.y * c.z - c.x * s.y * s.z;
	y = c.x * s.y * c.z + s.x * c.y * s.z;
	z = c.x * c.y * s.z - s.x * s.y * c.z;
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
	Tquat<T> q;
	q.x = -x;
	q.y = -y;
	q.z = -z;
	q.w = -w;
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
	T invNorm = T(1) / std::sqrt(x * x + y * y + z * z + w * w);
	x *= invNorm;
	y *= invNorm;
	z *= invNorm;
	w *= invNorm;
}

//constants
template<typename T> const Tquat<T> Tquat<T>::identity = Tquat<T>(1, 0, 0, 0);


// math function
template<typename T>
Tquat<T> operator*(const Tquat<T>& a, const Tquat<T>& b)
{
	Tquat<T> result;
	result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	result.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	result.z = a.w * b.z + a.x * b.y + a.z * b.w - a.y * b.x;
	result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	return result;
}

template<typename T>
Tvec4<T> operator*(const Tquat<T>& q, const Tvec4<T>& v)
{
	Tquat<T> vq = Tquat<T>(0, v.x, v.y, v.z);
	Tquat<T> r = q * vq * (-q);
	return Tvec4<T>(r.x, r.y, r.z, v.w);
}

template<typename T, typename T2>
Tquat<T> operator*(const Tquat<T>& a, const T2& scalar)
{
	return Tquat<T>(scalar * a.w, scalar * a.x, scalar * a.y, scalar * a.z);
}

template<typename T, typename T2>
Tquat<T> operator*(const T2& scalar, const Tquat<T>& a)
{
	return Tquat<T>(scalar * a.w, scalar * a.x, scalar * a.y, scalar * a.z);
}

template<typename T, typename T2>
Tquat<T> operator/(const Tquat<T>& a, const T2& scalar)
{
	return Tquat<T>(a.w / scalar, a.x / scalar, a.y / scalar, a.z / scalar);
}

template<typename T, typename T2>
Tquat<T> operator/(const T2& scalar, const Tquat<T>& a)
{
	return Tquat<T>(a.w / scalar, a.x / scalar, a.y / scalar, a.z / scalar);
}

template<typename T, typename T2>
Tquat<T> operator+(const Tquat<T>& a, const Tquat<T2>& b)
{
	return Tquat<T>(a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z);
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
		return Tquat<T>(lerp(a.w, z.w, t), lerp(a.x, z.x, t), lerp(a.y, z.y, t), lerp(a.z, z.z, t));
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

template<typename T>
Tquat<T> conjugate(const Tquat<T>& q)
{
	return Tquat<T>(q.w, -q.x, -q.y, -q.z);
}

template <typename T>
Tvec3<T> Tquat<T>::eulerAngles(Tquat<T> const& q)
{
	return Tvec3<T>(
		atan2(T(2) * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z),
		asin(clamp(T(-2) * (q.x * q.z - q.w * q.y), T(-1), T(1))),
		atan2(T(2) * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z)
	);
}