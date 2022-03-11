#pragma once
#include "Tvec3.h"

#ifdef SSE

#else

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
bool Tvec3<T>::operator==(const Tvec3& b)
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

// normalize
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
#endif