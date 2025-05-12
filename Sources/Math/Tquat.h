#pragma once

#include "Tvec4.h"

template<typename T>
class Tquat
{
public:
	union alignas(16)
	{
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };

#ifdef USE_SIMD
		typename simd4<T>::simdType v128;
#endif
	};

	// constructors
	Tquat() : x(T(0)), y(T(0)), z(T(0)), w(T(1)) {}

	template <typename A, typename B, typename C, typename D>
	Tquat(D _w, A _x, B _y, C _z) : x(T(_x)), y(T(_y)), z(T(_z)), w(T(_w)) {}

	template<typename T2>
	Tquat(const Tquat<T2>& q) : x((T)q.x), y((T)q.y), z((T)q.z), w((T)q.w) {}

	Tquat(const Tvec4<T>& from, const Tvec4<T>& to);

	Tquat(const T& angle, const Tvec4<T>& axis);
	Tquat(const T& angle, const Tvec3<T>& axis);

#ifdef USE_SIMD
	Tquat(typename simd4<T>::simdType _v128) : v128(_v128) {}
#endif

	template<typename T2, typename T3>
	Tquat(const Tvec3<T2>& v, T3 scalar) : x(v.x), y(v.y), z(v.z), w(scalar) {}

	Tquat(const Tvec3<T>& eulers);

	// component access
	T& operator[](int i) { return (&x)[i]; }
	const T& operator[](int i) const { return (&x)[i]; }

	// cast & swizzle
	Tvec3<T> xyz() const;
	Tvec4<T> xyzw() const;

	// operator
	Tquat operator-() const;
	void operator*=(const T& scalar);
	void operator/=(const T& scalar);

	bool operator==(const Tquat& b) const;
	bool operator!=(const Tquat& b) const;
	Tquat& operator=(const Tquat& b);

	// normalize
	void normalize();

	//constants
	static const Tquat identity;

	//math
	static Tquat lookAt(const Tvec4<T>& forward, Tvec4<T> up = Tvec4<T>(0, 1, 0, 0));
	static Tquat slerp(const Tquat& a, const Tquat& b, const T& t);
	static Tvec3<T> eulerAngles(const Tquat& q);
};

//#include "Tquat.hpp"
