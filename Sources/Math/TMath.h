#pragma once

#define USE_GLM

#ifdef USE_GLM

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define EPSILON 1E-6f
#define PI 3.14159265359f
#define DEG2RAD (PI / 180.f)
#define RAD2DEG (180.f / PI)

typedef glm::fvec4 vec4f;
typedef glm::fvec3 vec3f;
typedef glm::fvec2 vec2f;

typedef glm::fquat quatf;
typedef glm::fmat4 mat4f;

typedef glm::ivec4 vec4i;
typedef glm::ivec3 vec3i;
typedef glm::ivec2 vec2i;
typedef glm::bvec4 vec4b;
typedef glm::uvec4 vec4ui;


// vec4f function
template <typename T, glm::precision P>
T glm::tvec4<T, P>::getNorm() const
{
	return glm::length(*this);
}

template <typename T, glm::precision P>
void glm::tvec4<T, P>::normalize()
{
	*this = glm::normalize(*this);
}

template <typename T, glm::precision P>
T glm::tvec4<T, P>::getNorm2() const
{
	return glm::length2(*this);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::getNormal() const
{
	return glm::normalize(*this);
}

template <typename T, glm::precision P>
T glm::tvec4<T, P>::dot(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	return glm::dot(a, b);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::cross(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	return glm::cross(a, b);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::lerp(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b, const T& t)
{
	return glm::lerp(a, b, t);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::min(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	return glm::min(a, b);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::max(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	return glm::max(a, b);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::abs(const glm::tvec4<T, P>& a)
{
	return glm::abs(a);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::maskSelect(const glm::tvec4<bool, P>& mask, const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	glm::tvec4<T, P> result;
	for (int i = 0; i < 4; i++)
		result[i] = mask[i] ? a[i] : b[i];
	return result;
}

template <typename T, glm::precision P>
glm::tvec4<bool, P> glm::tvec4<T, P>::lessThan(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	return glm::lessThan(a, b);
}

template <typename T, glm::precision P>
glm::tvec4<bool, P> glm::tvec4<T, P>::greaterThan(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& b)
{
	return glm::greaterThan(a, b);
}

template <typename T, glm::precision P>
bool glm::tvec4<T, P>::any(const glm::tvec4<T, P>& b)
{
	return glm::any(b);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tvec4<T, P>::clamp(const glm::tvec4<T, P>& a, const glm::tvec4<T, P>& min, const glm::tvec4<T, P>& max)
{
	return glm::clamp(a, min, max);
}



// vec3f function
template <typename T, glm::precision P>
T glm::tvec3<T, P>::getNorm() const
{
	return glm::length(*this);
}

template <typename T, glm::precision P>
void glm::tvec3<T, P>::normalize()
{
	*this = glm::normalize(*this);
}

template <typename T, glm::precision P>
T glm::tvec3<T, P>::getNorm2() const
{
	return glm::length2(*this);
}

template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tvec3<T, P>::getNormal() const
{
	return glm::normalize(*this);
}

template <typename T, glm::precision P>
static T glm::tvec3<T, P>::dot(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b)
{
	return glm::dot(a, b);
}

template <typename T, glm::precision P>
static glm::tvec3<T, P> glm::tvec3<T, P>::cross(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b)
{
	return glm::cross(a, b);
}

template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tvec3<T, P>::lerp(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b, const T& t)
{
	return glm::lerp(a, b, t);
}

template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tvec3<T, P>::min(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b)
{
	return glm::min(a, b);
}

template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tvec3<T, P>::max(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b)
{
	return glm::min(a, b);
}

template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tvec3<T, P>::abs(const glm::tvec3<T, P>& a)
{
	return glm::abs(a);
}

template <typename T, glm::precision P>
glm::tvec3<bool, P> glm::tvec3<T, P>::lessThan(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b)
{
	return glm::lessThan(a, b);
}

template <typename T, glm::precision P>
glm::tvec3<bool, P> glm::tvec3<T, P>::greaterThan(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& b)
{
	return glm::greaterThan(a, b);
}

template <typename T, glm::precision P>
bool glm::tvec3<T, P>::any(const glm::tvec3<T, P>& b)
{
	return glm::any(b);
}

template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tvec3<T, P>::clamp(const glm::tvec3<T, P>& a, const glm::tvec3<T, P>& min, const glm::tvec3<T, P>& max)
{
	return glm::clamp(a, min, max);
}



// vec2f function
template <typename T, glm::precision P>
T glm::tvec2<T, P>::getNorm() const
{
	return glm::length(*this);
}

template <typename T, glm::precision P>
void glm::tvec2<T, P>::normalize()
{
	*this = glm::normalize(*this);
}

template <typename T, glm::precision P>
T glm::tvec2<T, P>::getNorm2() const
{
	return glm::length2(*this);
}

template <typename T, glm::precision P>
glm::tvec2<T, P> glm::tvec2<T, P>::getNormal() const
{
	return glm::normalize(*this);
}

template <typename T, glm::precision P>
static T glm::tvec2<T, P>::dot(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& b)
{
	return glm::dot(a, b);
}

template <typename T, glm::precision P>
glm::tvec2<T, P> glm::tvec2<T, P>::lerp(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& b, const T& t)
{
	return glm::lerp(a, b, t);
}

template <typename T, glm::precision P>
glm::tvec2<T, P> glm::tvec2<T, P>::min(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& b)
{
	return glm::min(a, b);
}

template <typename T, glm::precision P>
glm::tvec2<T, P> glm::tvec2<T, P>::max(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& b)
{
	return glm::min(a, b);
}

template <typename T, glm::precision P>
glm::tvec2<T, P> glm::tvec2<T, P>::abs(const glm::tvec2<T, P>& a)
{
	return glm::abs(a);
}

template <typename T, glm::precision P>
glm::tvec2<bool, P> glm::tvec2<T, P>::lessThan(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& b)
{
	return glm::lessThan(a, b);
}

template <typename T, glm::precision P>
glm::tvec2<bool, P> glm::tvec2<T, P>::greaterThan(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& b)
{
	return glm::greaterThan(a, b);
}

template <typename T, glm::precision P>
bool glm::tvec2<T, P>::any(const glm::tvec2<T, P>& b)
{
	return glm::any(b);
}

template <typename T, glm::precision P>
glm::tvec2<T, P> glm::tvec2<T, P>::clamp(const glm::tvec2<T, P>& a, const glm::tvec2<T, P>& min, const glm::tvec2<T, P>& max)
{
	return glm::clamp(a, min, max);
}



// quatf
template <typename T, glm::precision P>
void glm::tquat<T, P>::normalize()
{
	*this = glm::normalize(*this);
}


template <typename T, glm::precision P>
glm::tvec3<T, P> glm::tquat<T, P>::xyz() const
{
	return glm::tvec3<T, P>(x, y, z);
}

template <typename T, glm::precision P>
glm::tvec4<T, P> glm::tquat<T, P>::xyzw() const
{
	return glm::tvec4<T, P>(x, y, z, w);
}

template <typename T, glm::precision P>
glm::tquat<T, P> glm::tquat<T, P>::lookAt(const glm::tvec4<T, P>& forward, glm::tvec4<T, P> up)
{
	return glm::tquat<T, P>(glm::lookAt(glm::tvec3<T, P>(0,0,0), (glm::tvec3<T, P>)forward, (glm::tvec3<T, P>)up));
}

template <typename T, glm::precision P>
glm::tquat<T, P> glm::tquat<T, P>::slerp(const glm::tquat<T, P>& a, const glm::tquat<T, P>& b, const T& t)
{
	return glm::slerp(a, b, t);
}


// mat4f
template <typename T, glm::precision P>
GLM_FUNC_QUALIFIER glm::tmat4x4<T, P>::tmat4x4(glm::tquat<T, P> const& q)
{
	*this = glm::toMat4(q);
}

template <typename T, glm::precision P>
T glm::tmat4x4<T, P>::det() const
{
	return glm::determinant(*this);
}

template <typename T, glm::precision P>
glm::tquat<T, P> glm::tmat4x4<T, P>::extractRotation() const
{
	return tquat<T, P>(*this);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::translate(const glm::tmat4x4<T, P>& m, const glm::tvec4<T, P>& t)
{
	return glm::translate(m, (tvec3<T, P>)t);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::scale(const glm::tmat4x4<T, P>& m, const glm::tvec4<T, P>& s)
{
	return glm::scale(m, (tvec3<T, P>)s);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::rotate(const glm::tmat4x4<T, P>& m, const glm::tquat<T, P>& q)
{
	return glm::toMat4(q) * m;
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::rotate(const glm::tmat4x4<T, P>& m, const glm::tvec3<T, P> euler)
{
	return glm::eulerAngleXYZ(euler.x, euler.y, euler.z) * m;
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::TRS(const glm::tvec4<T, P>& t, const glm::tquat<T, P>& q, const glm::tvec4<T, P>& s)
{
	glm::tmat4x4<T, P> result = glm::toMat4(q);
	result[0] *= s[0];
	result[1] *= s[1];
	result[2] *= s[2];
	result[3] = t;
	return result;
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::fromTo(const glm::tvec4<T, P>& from, const glm::tvec4<T, P>& to)
{
	return glm::toMat4(glm::tquat<T, P>((tvec3<T, P>)from, (tvec3<T, P>)to));
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::lookAt(const glm::tvec4<T, P>& forward, glm::tvec4<T, P> up)
{
	return glm::orientation((tvec3<T, P>)forward, (tvec3<T, P>)up);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::ortho(const T left, const T right, const T bottom, const T top, const T zNear, const T zFar)
{
	return glm::ortho(left, right, bottom, top, zNear, zFar);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::inverse(const glm::tmat4x4<T, P>& m)
{
	return glm::inverse(m);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::transpose(const glm::tmat4x4<T, P>& m)
{
	return glm::transpose(m);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::perspective(T fovy, T aspect, T zNear, T zFar)
{
	return glm::perspective(fovy, aspect, zNear, zFar);
}

template <typename T, glm::precision P>
glm::tmat4x4<T, P> glm::tmat4x4<T, P>::eulerAngleZX(T z, T x)
{
	return glm::eulerAngleZX(z, x);
}


// scalar
template<typename T>
T lerp(const T& a, const T& b, const T& t) {
	return a + t * (b - a);
}

template<typename T>
T clamp(const T& a, const T& min, const T& max) {
	return a < min ? min : (a > max ? max : a);
}

#else

#include "Tmat4.h"

#endif





