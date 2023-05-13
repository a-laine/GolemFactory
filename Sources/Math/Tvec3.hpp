#pragma once
#include "Tvec3.h"


template<typename T>
Tvec3<T> Tvec3<T>::operator-() const
{
	return Tvec3<T>(-x, -y, -z);
}

template<typename T>
void Tvec3<T>::operator*=(const T& scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

template<typename T>
void Tvec3<T>::operator/=(const T& scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
}

template<typename T>
void Tvec3<T>::operator*=(const Tvec3& b)
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
}

template<typename T>
void Tvec3<T>::operator/=(const Tvec3& b)
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
}

template<typename T>
void Tvec3<T>::operator+=(const Tvec3& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
}

template<typename T>
void Tvec3<T>::operator-=(const Tvec3& b) const
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
}

template<typename T>
bool Tvec3<T>::operator==(const Tvec3& b) const
{
	return x == b.x && y == b.y && z == b.z;
}

template<typename T>
bool Tvec3<T>::operator!=(const Tvec3& b)
{
	return x != b.x && y != b.y && z != b.z;
}

template<typename T>
Tvec3<T>& Tvec3<T>::operator=(const Tvec3& b)
{
	x = b.x;
	y = b.y;
	z = b.z;
	return *this;
}

template<typename T>
T Tvec3<T>::getNorm() const
{
	sqrt(x * x + y * y + z * z);
}

template<typename T>
Tvec3<T> Tvec3<T>::getNormal() const
{
	T invnorm = T(1) / sqrt(x * x + y * y + z * z);
	return Tvec3<T>(x * invnorm, y * invnorm, z * invnorm);
}

template<typename T>
void Tvec3<T>::normalize()
{
	T invnorm = T(1) / sqrt(x * x + y * y + z * z);
	x *= invnorm;
	y *= invnorm;
	z *= invnorm;
}

template<typename T, typename T2>
Tvec3<T> operator+(const Tvec3<T>& a, const Tvec3<T2>& b)
{
	return Tvec3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template<typename T, typename T2>
Tvec3<T> operator-(const Tvec3<T>& a, const Tvec3<T2>& b)
{
	return Tvec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template<typename T, typename T2>
Tvec3<T> operator*(const Tvec3<T>& a, const Tvec3<T2>& b)
{
	return Tvec3<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}

template<typename T, typename T2>
Tvec3<T> operator/(const Tvec3<T>& a, const Tvec3<T2>& b)
{
	return Tvec3<T>(a.x / b.x, a.y / b.y, a.z / b.z);
}

template<typename T, typename T2>
Tvec3<T> operator*(const T2& scalar, const Tvec3<T>& a)
{
	return Tvec3<T>(scalar * a.x, scalar * a.y, scalar * a.z);
}

template<typename T, typename T2>
Tvec3<T> operator*(const Tvec3<T>& a, const T2& scalar)
{
	return Tvec3<T>(scalar * a.x, scalar * a.y, scalar * a.z);
}

template<typename T, typename T2>
Tvec3<T> operator/(const Tvec3<T>& a, const T2& scalar)
{
	return Tvec3<T>(a.x / scalar, a.y / scalar, a.z / scalar);
}

template<typename T> const Tvec3<T> Tvec3<T>::zero = Tvec3<T>(T(0));
template<typename T> const Tvec3<T> Tvec3<T>::one = Tvec3<T>(T(1));







/// Math function
template<typename T>
T Tvec3<T>::dot(const Tvec3<T>& a, const Tvec3<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
Tvec3<T> Tvec3<T>::cross(const Tvec3<T>& a, const Tvec3<T>& b)
{
	return Tvec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template<typename T>
Tvec3<T> Tvec3<T>::lerp(const Tvec3<T>& a, const Tvec3<T>& b, const T& t)
{
	return a + t * (b - a);
}