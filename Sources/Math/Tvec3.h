#pragma once

#include "Tvec2.h"

template<typename T>
class Tvec3
{
	public:
		union
		{
			struct { T x, y, z; };
			struct { T r, g, b; };
		};

		// constructors
		Tvec3() : x(T(0)), y(T(0)), z(T(0)) {}

		template <typename A, typename B, typename C>
		Tvec3(A _x, B _y, C _z) : x(T(_x)), y(T(_y)), z(T(_z)) {}

		template<typename T2>
		Tvec3(T2 v = T2(0)) : x(T(v)), y(T(v)), z(T(v)) {}

		template<typename T2>
		Tvec3(const Tvec3<T2>& v) : x(v.x), y(v.y), z(v.z) {}

		template<typename T2, typename T3>
		Tvec3(const Tvec2<T2>& v, T3 scalar = T3(0)) : x(v.x), y(v.y), z(scalar) {}

		// component access
		T& operator[](int i) { return (&x)[i]; }
		const T& operator[](int i) const { return (&x)[i]; }

		// cast
		operator Tvec2<T>() const { return Tvec2<T>(x, y); }

		// operator
		Tvec3 operator-() const;
		void operator*=(const T& scalar);
		void operator/=(const T& scalar);
		void operator*=(const Tvec3& b);
		void operator/=(const Tvec3& b);
		void operator+=(const Tvec3& b);
		void operator-=(const Tvec3& b) const;
		bool operator==(const Tvec3& b) const;
		bool operator!=(const Tvec3& b);
		Tvec3& operator=(const Tvec3& b);

		// normalize
		T getNorm() const;
		Tvec3 getNormal() const;
		void normalize();

		//constants
		static const Tvec3 zero;
		static const Tvec3 one;
};

#include "Tvec3.hpp"