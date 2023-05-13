#pragma once


#include "Tvec3.h"


template<typename T>
class Tvec4
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
		Tvec4() : x(T(0)), y(T(0)), z(T(0)), w(T(0)) {}

		template <typename A, typename B, typename C, typename D>
		Tvec4(A _x, B _y, C _z, D _w) : x(T(_x)), y(T(_y)), z(T(_z)), w(T(_w)) {}

		template<typename T2>
		Tvec4(T2 v = T2(0)) : x(T(v)), y(T(v)), z(T(v)), w(T(v)) {}

		template<typename T2>
		Tvec4(const Tvec4<T2>& v) : x((T)v.x), y((T)v.y), z((T)v.z), w((T)v.w) {}

		#ifdef USE_SIMD
		Tvec4(typename simd4<T>::simdType _v128) : v128(_v128) {}
		#endif

		template<typename T2, typename T3>
		Tvec4(const Tvec2<T2>& v1, const Tvec2<T3>& v2) : x(v1.x), y(v1.y), z(v2.x), w(v2.y) {}

		template<typename T2, typename T3>
		Tvec4(const Tvec3<T2>& v, T3 scalar = T3(0)) : x(v.x), y(v.y), z(v.z), w(scalar) {}

		// component access
		T& operator[](int i) { return (&x)[i]; }
		const T& operator[](int i) const { return (&x)[i]; }

		// cast & swizzle
		operator Tvec3<T>() const { return Tvec3<T>(x, y, z); }
		operator Tvec2<T>() const { return Tvec2<T>(x, y); }
		Tvec3<T> xyz() const { return Tvec3<T>(x, y, z); }

		// operator
		Tvec4 operator-() const;
		void operator*=(const T& scalar);
		void operator/=(const T& scalar);
		void operator+=(const Tvec4& b);
		void operator-=(const Tvec4& b);
		void operator*=(const Tvec4& b);
		void operator/=(const Tvec4& b);

		bool operator==(const Tvec4& b) const;
		bool operator!=(const Tvec4& b) const;
		Tvec4& operator=(const Tvec4& b);

		// normalize
		T getNorm() const;
		T getNorm2() const;
		Tvec4 getNorm4() const;
		Tvec4 getNormal() const;
		void normalize();

		//constants
		static const Tvec4 zero;
		static const Tvec4 one;

		// math
		static T dot(const Tvec4& a, const Tvec4& b);
		static Tvec4 dot4(const Tvec4& a, const Tvec4& b);
		static Tvec4 cross(const Tvec4& a, const Tvec4& b);
		static Tvec4 lerp(const Tvec4& a, const Tvec4& b, const T& t);
		static Tvec4 min(const Tvec4& a, const Tvec4& b);
		static Tvec4 max(const Tvec4& a, const Tvec4& b);
		static Tvec4 abs(const Tvec4& a);
		static Tvec4<bool> lessThan(const Tvec4& a, const Tvec4& b);
		static Tvec4<bool> greaterThan(const Tvec4& a, const Tvec4& b);
		static bool any(const Tvec4& b);
		static Tvec4 clamp(const Tvec4& a, const Tvec4& min, const Tvec4& max);
};

#include "Tvec4.hpp"
