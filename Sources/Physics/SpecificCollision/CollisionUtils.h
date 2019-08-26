#pragma once

#include <algorithm>
#include <iostream>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>

#define COLLISION_EPSILON 0.0001f


//	Private field
namespace
{
	//	Utils
	inline float projectHalfBox(const glm::vec3& axis, const glm::vec3& boxHalfSize)
	{
		//	axis is in absolute base
		//	boxHalfSize is in box local space (origin is box center)
		//	equivalent to :  glm::dot(boxHalfSize, glm::abs(axis))
		return std::abs(boxHalfSize.x*axis.x) + std::abs(boxHalfSize.y*axis.y) + std::abs(boxHalfSize.z*axis.z);
	}
	inline float projectHalfCapsule(const glm::vec3& axis, const glm::vec3& capsuleSegment, const float& capsuleRadius)
	{
		//	axis is in absolute base
		return capsuleRadius + 0.5f * std::abs(glm::dot(capsuleSegment, axis));
	}
	inline float projectTriangle(const glm::vec3& axis, const glm::vec3& edge1, const glm::vec3& edge2)
	{
		return std::max(0.f, std::max(glm::dot(axis, edge1), glm::dot(axis, edge2)));
	}

	inline glm::vec3 getSegmentClosestPoint(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& point)
	{
		if (segment2 == segment1) return segment1;
		const glm::vec3 s = segment2 - segment1;
		const float ss = glm::length2(s);
		return segment1 + glm::clamp(glm::dot(point - segment1, s) / ss, 0.f, 1.f) * s;
	}
	inline std::pair<glm::vec3, glm::vec3> getSegmentsClosestSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
	{
		//http://geomalgorithms.com/a07-_distance.html
		glm::vec3 u = segment1b - segment1a;
		glm::vec3 v = segment2b - segment2a;
		glm::vec3 w = segment2a - segment1a;

		float a = glm::length2(u);
		float b = glm::dot(u, v);
		float c = glm::length2(v);
		float d = glm::dot(u, w);
		float e = glm::dot(v, w);
		float D = a*c - b*b;

		float t1, t2;
		if (D < COLLISION_EPSILON*COLLISION_EPSILON)
		{
			t1 = 0.0;
			t2 = -(b>c ? d / b : e / c);
		}
		else
		{
			t1 = -(b*e - c*d) / D;
			t2 = -(a*e - b*d) / D;
		}

		if (t1 < 0.f) // t1 = 0 is a quadratic limit check
		{
			t1 = 0.f;
			t2 = -(e + b) / c; //e / c;
		}
		else if (t1 > 1.f) // t1 = 1 is a quadratic limit check
		{
			t1 = 1.f;
			t2 = -e / c; //(e + b) / c;
		}

		if (t2 < 0.f)
		{
			t2 = 0.f;
			t1 = glm::clamp(-d / a, 0.f, 1.f);
		}
		else if (t2 > 1.f)
		{
			t2 = 1.f;
			t1 = glm::clamp((-d + b) / a, 0.f, 1.f);
		}

		//t1 = glm::clamp(t1, 0.f, 1.f);
		//t2 = glm::clamp(t2, 0.f, 1.f);

		/*if (t1 < 0.f) return std::pair<glm::vec3, glm::vec3>(segment1a, getSegmentClosestPoint(segment2a, segment2b, segment1a));
		else if (t1 > 1.f) return std::pair<glm::vec3, glm::vec3>(segment1b, getSegmentClosestPoint(segment2a, segment2b, segment1b));
		else if (t2 < 0.f) return std::pair<glm::vec3, glm::vec3>(getSegmentClosestPoint(segment1a, segment1b, segment2a), segment2a);
		else if (t2 > 1.f) return std::pair<glm::vec3, glm::vec3>( getSegmentClosestPoint(segment1a, segment1b, segment2b), segment2b);
		else*/ return std::pair<glm::vec3, glm::vec3>(segment1a + u*t1, segment2a + v*t2);
	}
	inline glm::vec2 getBarycentricCoordinates(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& point)
	{
		float crossDot = glm::dot(v1, v2);
		float magnitute = glm::dot(v1, v1)*glm::dot(v2, v2) - crossDot*crossDot;
		glm::vec2 bary;

		bary.x = (glm::dot(v2, v2) * glm::dot(point, v1) - crossDot * glm::dot(point, v2)) / magnitute;
		bary.y = (glm::dot(v1, v1) * glm::dot(point, v2) - crossDot * glm::dot(point, v1)) / magnitute;
		return bary;
	}
	//
};
//