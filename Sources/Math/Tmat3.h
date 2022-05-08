#pragma once

#include "Tvec3.h"

template<typename T>
class Tmat3
{
	public:
		union
		{
			struct { T m[3][3]; };
		};


		// constructors
		Tmat3();

		template   <typename T00, typename T10, typename T20,
					typename T01, typename T11, typename T21,
					typename T02, typename T12, typename T22>
		Tmat3(T00 m00, T01 m01, T02 m02, T10 m10, T11 m11, T12 m12, T20 m20, T21 m21, T22 m22);

		template<typename T2>
		Tmat3(T2 v);

		template<typename T2>
		Tmat3(const Tmat3<T2>& b);


		// component access
		T& operator[](int i) { return m; }
		const T& operator[](int i) const { return m; }

		// operator
		Tvec3 operator-() const;
		void operator*=(const T& scalar);
		void operator/=(const T& scalar);
		void operator*=(const Tmat3& b);
		void operator/=(const Tmat3& b);
		void operator+=(const Tmat3& b);
		void operator-=(const Tmat3& b) const;
		bool operator==(const Tmat3& b) const;
		bool operator!=(const Tmat3& b);
		Tmat3& operator=(const Tmat3& b);

		// functions
		T getDeterminant() const;
		Tmat3 getInverse() const;
		void inverse();

		// constants
		static const Tmat3 zero;
		static const Tmat3 identity;
};