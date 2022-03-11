#pragma once

#include "Tvec2.h"

template<typename T>
class Tvec3
{
public:
	using type = T;
	union
	{
		struct { T x, y, z; };
	};

	// constructors
	Tvec3() : x(T(0)), y(T(0)), z(T(0)) {}

	template<typename T2>
	Tvec3(T2 _x, T2 _y, T2 _z) : x(T(_x)), y(T(_y)), z(T(_z)) {}

	template <typename A, typename B, typename C>
	Tvec3(A _x, B _y, C _z) : x(T(_x)), y(T(_y)), z(T(_z)) {}

	template<typename T2>
	Tvec3(T2 v = T2(0)) : x(T(v)), y(T(v)), z(T(v)) {}

	template<typename T2>
	Tvec3(Tvec3<T2> const& v) : x(v.x), y(v.y), z(v.z) {}

	template<typename T2, typename T3>
	Tvec3(const Tvec2<T2>& v, T3 scalar = T3(0)) : x(v.x), y(v.y), z(scalar) {}

	// component access
	T& operator[](int i) { return (&x)[i]; }
	T const& operator[](int i) const { return (&x)[i]; }

	// cast
	operator Tvec2<T>() const { return Tvec2<T>(x, y); }

	// operator
	void operator*=(const T& scalar);
	void operator/=(const T& scalar);
	bool operator==(const Tvec3& b);
	bool operator!=(const Tvec3& b);
	Tvec3& operator=(const Tvec3& b);

	// normalize
	T getNorm() const;
	Tvec3 getNormal() const;
	void normalize();
};

template<typename T, typename T2>
Tvec3<T> operator+(const Tvec3<T>& a, const Tvec3<T>& b);

template<typename T, typename T2>
Tvec3<T> operator-(const Tvec3<T>& a, const Tvec3<T>& b);

template<typename T, typename T2>
Tvec3<T> operator*(const Tvec3<T>& a, const Tvec3<T>& b);

template<typename T, typename T2>
Tvec3<T> operator*(const T2& scalar, const Tvec3<T>& a);

template<typename T, typename T2>
Tvec3<T> operator*(const Tvec3<T>& a, const T2& scalar);

template<typename T, typename T2>
Tvec3<T> operator/(const Tvec3<T>& a, const T2& scalar);

#include "Tvec3.hpp"