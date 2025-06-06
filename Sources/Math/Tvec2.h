#pragma once

#include "Ttypes.h"

// __m128d vec2d


template<typename T>
class Tvec2
{
	public:
		union
		{
			struct { T x, y; };
			struct { T u, v; };
		};

		// constructors
		Tvec2() : x(T(0)), y(T(0)) {}

		template <typename A, typename B>
		Tvec2(A _x, B _y) : x(T(_x)), y(T(_y)) {}

		template<typename T2>
		Tvec2(T2 v = T2(0)) : x(T(v)), y(T(v)) {}

		template<typename T2>
		Tvec2(const Tvec2<T2>& v) : x(T(v.x)), y(T(v.y)) {}

		// component access
		T& operator[](int i) { return (&x)[i]; }
		const T& operator[](int i) const { return (&x)[i]; }

		// operator
		Tvec2 operator-() const;
		void operator*=(const T& scalar);
		void operator/=(const T& scalar);
		void operator*=(const Tvec2& b);
		void operator/=(const Tvec2& b);
		void operator+=(const Tvec2& b);
		void operator-=(const Tvec2& b);
		bool operator==(const Tvec2& b) const;
		bool operator!=(const Tvec2& b) const;
		Tvec2& operator=(const Tvec2& b);
		Tvec2<bool> operator<(const Tvec2& b) const;
		Tvec2<bool> operator>(const Tvec2& b) const;

		// norm & normalize
		T getNorm() const;
		T getNorm2() const;
		Tvec2 getNormal() const;
		void normalize();

		//constants
		static const Tvec2 zero;
		static const Tvec2 one;

		//math
		static Tvec2 min(const Tvec2& a, const Tvec2& b);
		static Tvec2 max(const Tvec2& a, const Tvec2& b);
		static Tvec2 clamp(const Tvec2& a, const Tvec2& min, const Tvec2& max);
};

//#include "Tvec2.hpp"