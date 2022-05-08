#pragma once
#include "Tvec2.h"




template<typename T>
Tvec2<T> Tvec2<T>::operator-() const
{
	return Tvec2<T>(-x, -y);
}

template<typename T>
void Tvec2<T>::operator*=(const T& scalar)
{
	x *= scalar; 
	y *= scalar;
}

template<typename T>
void Tvec2<T>::operator/=(const T& scalar)
{
	x /= scalar;
	y /= scalar;
}

template<typename T>
void Tvec2<T>::operator*=(const Tvec2<T>& b)
{
	x *= b.x;
	y *= b.y;
}

template<typename T>
void Tvec2<T>::operator/=(const Tvec2<T>& b)
{
	x /= b.x;
	y /= b.y;
}

template<typename T>
void Tvec2<T>::operator+=(const Tvec2<T>& b)
{
	x += b.x;
	y += b.y;
}

template<typename T>
void Tvec2<T>::operator-=(const Tvec2<T>& b)
{
	x -= b.x;
	y -= b.y;
}

template<typename T>
bool Tvec2<T>::operator==(const Tvec2<T>& b) const
{
	return x == b.x && y == b.y;
}

template<typename T>
bool Tvec2<T>::operator!=(const Tvec2<T>& b) const
{
	return x != b.x && y != b.y;
}

template<typename T>
Tvec2<T>& Tvec2<T>::operator=(const Tvec2<T>& b)
{
	x = b.x;
	y = b.y;
	return *this;
}

template<typename T>
T Tvec2<T>::getNorm() const
{
	return sqrt(x * x + y * y);
}

template<typename T>
Tvec2<T> Tvec2<T>::getNormal() const
{
	T invnorm = T(1) / sqrt(x * x + y * y);
	return Tvec2<T>(x * invnorm, y * invnorm);
}

template<typename T>
void Tvec2<T>::normalize()
{
	T invnorm = T(1) / sqrt(x * x + y * y);
	x *= invnorm;
	y *= invnorm;
}

template<typename T, typename T2>
Tvec2<T> operator+(const Tvec2<T>& a, const Tvec2<T2>& b)
{
	return Tvec2<T>(a.x + b.x, a.y + b.y);
}

template<typename T, typename T2>
Tvec2<T> operator-(const Tvec2<T>& a, const Tvec2<T2>& b)
{
	return Tvec2<T>(a.x - b.x, a.y - b.y);
}

template<typename T, typename T2>
Tvec2<T> operator*(const Tvec2<T>& a, const Tvec2<T2>& b)
{
	return Tvec2<T>(a.x * b.x, a.y * b.y);
}

template<typename T, typename T2>
Tvec2<T> operator/(const Tvec2<T>& a, const Tvec2<T2>& b)
{
	return Tvec2<T>(a.x / b.x, a.y / b.y);
}

template<typename T, typename T2>
Tvec2<T> operator*(const T2& scalar, const Tvec2<T>& a)
{
	return Tvec2<T>(scalar * a.x, scalar * a.y);
}

template<typename T, typename T2>
Tvec2<T> operator*(const Tvec2<T>& a, const T2& scalar)
{
	return Tvec2<T>(scalar * a.x, scalar * a.y);
}

template<typename T, typename T2>
Tvec2<T> operator/(const Tvec2<T>& a, const T2& scalar)
{
	return Tvec2<T>(a.x / scalar, a.y / scalar);
}

template<typename T> const Tvec2<T> Tvec2<T>::zero = Tvec2<T>(0);
template<typename T> const Tvec2<T> Tvec2<T>::one = Tvec2<T>(1);