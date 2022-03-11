#pragma once

#include "Tvec.h"

// __m128d vec2d


template<typename T>
class Tvec2
{
public:
	using type = T;
	union
	{
		struct { T x, y; };
	};

	// constructors
	Tvec2() : x(T(0)), y(T(0)) {}

	template<typename T2>
	Tvec2(T2 _x, T2 _y) : x(T(_x)), y(T(_y)) {}

	template <typename A, typename B>
	Tvec2(A _x, B _y) : x(_x), y(_y) {}

	template<typename T2>
	Tvec2(T2 v = T2(0)) : x(v), y(v) {}

	template<typename T2>
	Tvec2(Tvec2<T2> const& v) : x(v.x), y(v.y) {}

	// component access
	T& operator[](int i) { return (&x)[i]; }
	T const& operator[](int i) const { return (&x)[i]; }

	// operator
	void operator*=(const T& scalar);
	void operator/=(const T& scalar);
	bool operator==(const Tvec2& b);
	bool operator!=(const Tvec2& b);
	Tvec2& operator=(const Tvec2& b);

	// norm & normalize
	T getNorm() const;
	Tvec2 getNormal() const;
	void normalize();
};

template<typename T, typename T2>
Tvec2<T> operator+(const Tvec2<T>& a, const Tvec2<T2>& b);

template<typename T, typename T2>
Tvec2<T> operator-(const Tvec2<T>& a, const Tvec2<T2>& b);

template<typename T, typename T2>
Tvec2<T> operator*(const Tvec2<T>& a, const Tvec2<T2>& b);

template<typename T, typename T2>
Tvec2<T> operator*(const Tvec2<T>& a, const T2& scalar);

template<typename T, typename T2>
Tvec2<T> operator*(const T2& scalar, const Tvec2<T>& a);

template<typename T, typename T2>
Tvec2<T> operator/(const Tvec2<T>& a, const T2& scalar);

#include "Tvec2.hpp"