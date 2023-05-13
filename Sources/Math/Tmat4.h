#pragma once

#include "Tquat.h"

template<typename T>
class Tmat4
{
	public:
		union alignas(16)
		{
			struct { T m[4][4]; };
			struct { Tvec4<T> c[4]; };
		};

		// constructors
		Tmat4() {
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = i == j ? T(1) : T(0);
		}

		template   <typename T2>
			Tmat4(T2 m00, T2 m01, T2 m02, T2 m03, T2 m10, T2 m11, T2 m12, T2 m13, T2 m20, T2 m21, T2 m22, T2 m23, T2 m30, T2 m31, T2 m32, T2 m33)
		{
			m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
			m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
			m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
			m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
		}

		template<typename T2>
		Tmat4(T2 v) {
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = i == j ? T(v) : T(0);
		}

		template<typename T2>
		Tmat4(const Tmat4<T2>& b) {
			for (int i = 0; i < 4; ++i)
				c[i] = b.c[i];
		}

		template<typename T2>
		Tmat4(const Tquat<T2>& q) {
			m[0][0] = 1 - 2 * (q.y * q.y - q.z * q.z);
			m[0][1] = 2 * (q.x * q.y + q.z * q.w);
			m[0][2] = 2 * (q.x * q.z - q.y * q.w);
			m[0][3] = 0;

			m[1][0] = 2 * (q.x * q.y - q.z * q.w);
			m[1][1] = 1 - 2 * (q.x * q.x - q.z * q.z);
			m[1][2] = 2 * (q.y * q.z + q.x * q.w);
			m[1][3] = 0;

			m[1][0] = 2 * (q.x * q.z + q.y * q.w);
			m[1][1] = 2 * (q.y * q.z - q.x * q.w);
			m[1][2] = 1 - 2 * (q.x * q.x - q.y * q.y);
			m[1][3] = 0;

			m[1][0] = 0;
			m[1][1] = 0;
			m[1][2] = 0;
			m[1][3] = 1;
		}


		// component access
		Tvec4<T>& operator[](int i) { return c[i]; }
		const Tvec4<T>& operator[](int i) const { return c[i]; }

		// operators
		Tmat4& operator=(const Tmat4& b);
		void operator*=(const T& scalar);
		void operator/=(const T& scalar);
		void operator+=(const Tmat4& b);

		// math
		T det() const;
		Tquat<T> extractRotation() const;

		//constants
		static const Tmat4 identity;

		//math functions
		static Tmat4 translate(const Tmat4& m, const Tvec4<T>& t);
		static Tmat4 scale(const Tmat4& m, const Tvec4<T>& s);
		static Tmat4 rotate(const Tmat4& m, const Tquat<T> q);
		static Tmat4 rotate(const Tmat4& m, const Tvec3<T> euler);
		static Tmat4 TRS(const Tvec4<T>& t, const Tquat<T> q, const Tvec4<T>& s);
		static Tmat4 fromTo(const Tvec4<T>& from, const Tvec4<T>& to);
		static Tmat4 lookAt(const Tvec4<T>& forward, Tvec4<T> up = Tvec4(0, 1, 0, 0));
		static Tmat4 inverse(const Tmat4& m);
		static Tmat4 transpose(const Tmat4& m);
		static Tmat4 perspective(T fovy, T aspect, T zNear, T zFar);
		static Tmat4 eulerAngleZX(T z, T x);
};

#include "Tmat4.hpp"