///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// Restrictions:
///		By making use of the Software for military purposes, you choose to make
///		a Bunny unhappy.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file glm/detail/type_vec3.hpp
/// @date 2008-08-22 / 2011-06-15
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "type_vec.hpp"
#ifdef GLM_SWIZZLE
#	if GLM_HAS_ANONYMOUS_UNION
#		include "_swizzle.hpp"
#	else
#		include "_swizzle_func.hpp"
#	endif
#endif //GLM_SWIZZLE
#include <cstddef>

namespace glm
{
	template <typename T, precision P = defaultp>
	struct tvec3
	{
		// -- Implementation detail --

		typedef tvec3<T, P> type;
		typedef tvec3<bool, P> bool_type;
		typedef T value_type;

#		ifdef GLM_META_PROG_HELPERS
			static GLM_RELAXED_CONSTEXPR length_t components = 3;
			static GLM_RELAXED_CONSTEXPR precision prec = P;
#		endif//GLM_META_PROG_HELPERS

		// -- Data --

#		if GLM_HAS_ANONYMOUS_UNION
			union
			{
				struct{ T x, y, z; };
				struct{ T r, g, b; };
				struct{ T s, t, p; };

#				ifdef GLM_SWIZZLE
					_GLM_SWIZZLE3_2_MEMBERS(T, P, tvec2, x, y, z)
					_GLM_SWIZZLE3_2_MEMBERS(T, P, tvec2, r, g, b)
					_GLM_SWIZZLE3_2_MEMBERS(T, P, tvec2, s, t, p)
					_GLM_SWIZZLE3_3_MEMBERS(T, P, tvec3, x, y, z)
					_GLM_SWIZZLE3_3_MEMBERS(T, P, tvec3, r, g, b)
					_GLM_SWIZZLE3_3_MEMBERS(T, P, tvec3, s, t, p)
					_GLM_SWIZZLE3_4_MEMBERS(T, P, tvec4, x, y, z)
					_GLM_SWIZZLE3_4_MEMBERS(T, P, tvec4, r, g, b)
					_GLM_SWIZZLE3_4_MEMBERS(T, P, tvec4, s, t, p)
#				endif//GLM_SWIZZLE
			};
#		else
			union { T x, r, s; };
			union { T y, g, t; };
			union { T z, b, p; };

#			ifdef GLM_SWIZZLE
				GLM_SWIZZLE_GEN_VEC_FROM_VEC3(T, P, tvec3, tvec2, tvec3, tvec4)
#			endif//GLM_SWIZZLE
#		endif//GLM_LANG

		// -- Component accesses --

#		ifdef GLM_FORCE_SIZE_FUNC
			/// Return the count of components of the vector
			typedef size_t size_type;
			GLM_FUNC_DECL GLM_CONSTEXPR size_type size() const;

			GLM_FUNC_DECL T & operator[](size_type i);
			GLM_FUNC_DECL T const & operator[](size_type i) const;
#		else
			/// Return the count of components of the vector
			typedef length_t length_type;
			GLM_FUNC_DECL GLM_CONSTEXPR length_type length() const;

			GLM_FUNC_DECL T & operator[](length_type i);
			GLM_FUNC_DECL T const & operator[](length_type i) const;
#		endif//GLM_FORCE_SIZE_FUNC

		// -- Implicit basic constructors --

		GLM_FUNC_DECL tvec3() GLM_DEFAULT_CTOR;
		GLM_FUNC_DECL tvec3(tvec3<T, P> const & v) GLM_DEFAULT;
		template <precision Q>
		GLM_FUNC_DECL tvec3(tvec3<T, Q> const & v);

		// -- Explicit basic constructors --

		GLM_FUNC_DECL explicit tvec3(ctor);
		GLM_FUNC_DECL explicit tvec3(T const & scalar);
		GLM_FUNC_DECL tvec3(T const & a, T const & b, T const & c);

		// -- Conversion scalar constructors --

		/// Explicit converions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename A, typename B, typename C>
		GLM_FUNC_DECL tvec3(A const & a, B const & b, C const & c);
		template <typename A, typename B, typename C>
		GLM_FUNC_DECL tvec3(tvec1<A, P> const & a, tvec1<B, P> const & b, tvec1<C, P> const & c);

		// -- Conversion vector constructors --

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename A, typename B, precision Q>
		GLM_FUNC_DECL explicit tvec3(tvec2<A, Q> const & a, B const & b);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename A, typename B, precision Q>
		GLM_FUNC_DECL explicit tvec3(tvec2<A, Q> const & a, tvec1<B, Q> const & b);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename A, typename B, precision Q>
		GLM_FUNC_DECL explicit tvec3(A const & a, tvec2<B, Q> const & b);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename A, typename B, precision Q>
		GLM_FUNC_DECL explicit tvec3(tvec1<A, Q> const & a, tvec2<B, Q> const & b);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename U, precision Q>
		GLM_FUNC_DECL explicit tvec3(tvec4<U, Q> const & v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template <typename U, precision Q>
		GLM_FUNC_DECL GLM_EXPLICIT tvec3(tvec3<U, Q> const & v);

		// -- Swizzle constructors --

#		if GLM_HAS_ANONYMOUS_UNION && defined(GLM_SWIZZLE)
			template <int E0, int E1, int E2>
			GLM_FUNC_DECL tvec3(detail::_swizzle<3, T, P, tvec3<T, P>, E0, E1, E2, -1> const & that)
			{
				*this = that();
			}

			template <int E0, int E1>
			GLM_FUNC_DECL tvec3(detail::_swizzle<2, T, P, tvec2<T, P>, E0, E1, -1, -2> const & v, T const & scalar)
			{
				*this = tvec3<T, P>(v(), scalar);
			}

			template <int E0, int E1>
			GLM_FUNC_DECL tvec3(T const & scalar, detail::_swizzle<2, T, P, tvec2<T, P>, E0, E1, -1, -2> const & v)
			{
				*this = tvec3<T, P>(scalar, v());
			}
#		endif// GLM_HAS_ANONYMOUS_UNION && defined(GLM_SWIZZLE)

		// -- Unary arithmetic operators --

		GLM_FUNC_DECL tvec3<T, P> & operator=(tvec3<T, P> const & v) GLM_DEFAULT;

		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator+=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator+=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator+=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator-=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator-=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator-=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator*=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator*=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator*=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator/=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator/=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator/=(tvec3<U, P> const & v);

		// -- Increment and decrement operators --

		GLM_FUNC_DECL tvec3<T, P> & operator++();
		GLM_FUNC_DECL tvec3<T, P> & operator--();
		GLM_FUNC_DECL tvec3<T, P> operator++(int);
		GLM_FUNC_DECL tvec3<T, P> operator--(int);

		// -- Unary bit operators --

		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator%=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator%=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator%=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator&=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator&=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator&=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator|=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator|=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator|=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator^=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator^=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator^=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator<<=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator<<=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator<<=(tvec3<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator>>=(U scalar);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator>>=(tvec1<U, P> const & v);
		template <typename U>
		GLM_FUNC_DECL tvec3<T, P> & operator>>=(tvec3<U, P> const & v);


		// personal extension
		T getNorm() const;
		T getNorm2() const;
		tvec3<T, P> getNormal() const;
		void normalize();

		static T dot(const tvec3<T, P>& a, const tvec3<T, P>& b);
		static tvec3<T, P> cross(const tvec3<T, P>& a, const tvec3<T, P>& b);
		static tvec3<T, P> lerp(const tvec3<T, P>& a, const tvec3<T, P>& b, const T& t);
		static tvec3<T, P> min(const tvec3<T, P>& a, const tvec3<T, P>& b);
		static tvec3<T, P> max(const tvec3<T, P>& a, const tvec3<T, P>& b);
		static tvec3<T, P> abs(const tvec3<T, P>& a);
		static tvec3<bool, P> lessThan(const tvec3<T, P>& a, const tvec3<T, P>& b);
		static tvec3<bool, P> greaterThan(const tvec3<T, P>& a, const tvec3<T, P>& b);
		static bool any(const tvec3<T, P>& b);
		static tvec3<T, P> clamp(const tvec3<T, P>& a, const tvec3<T, P>& min, const tvec3<T, P>& max);

		static tvec3<T, P> zero;
		static tvec3<T, P> one;
	};

	template <typename T, precision P>
	tvec3<T, P> tvec3<T, P>::zero = tvec3<T, P>(T(0));
	template <typename T, precision P>
	tvec3<T, P> tvec3<T, P>::one = tvec3<T, P>(T(1));

	// -- Unary operators --

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator+(tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator-(tvec3<T, P> const & v);

	// -- Binary operators --

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator+(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator+(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator+(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator+(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator+(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator-(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator-(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator-(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator-(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator-(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator*(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator*(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator*(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator*(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator*(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator/(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator/(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator/(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator/(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator/(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator%(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator%(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator%(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator%(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator%(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator&(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator&(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator&(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator&(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator&(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator|(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator|(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator|(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator|(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator|(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator^(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator^(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator^(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator^(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator^(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator<<(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator<<(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator<<(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator<<(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator<<(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator>>(tvec3<T, P> const & v, T const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator>>(tvec3<T, P> const & v, tvec1<T, P> const & scalar);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator>>(T const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator>>(tvec1<T, P> const & scalar, tvec3<T, P> const & v);

	template <typename T, precision P>
	GLM_FUNC_DECL tvec3<T, P> operator>>(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P> 
	GLM_FUNC_DECL tvec3<T, P> operator~(tvec3<T, P> const & v);

	// -- Boolean operators --

	template <typename T, precision P>
	GLM_FUNC_DECL bool operator==(tvec3<T, P> const & v1, tvec3<T, P> const & v2);

	template <typename T, precision P>
	GLM_FUNC_DECL bool operator!=(tvec3<T, P> const & v1, tvec3<T, P> const & v2);
}//namespace glm

#ifndef GLM_EXTERNAL_TEMPLATE
#include "type_vec3.inl"
#endif//GLM_EXTERNAL_TEMPLATE
