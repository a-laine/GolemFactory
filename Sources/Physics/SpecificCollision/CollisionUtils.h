#pragma once

#include <algorithm>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>


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
		const glm::vec3 s = segment2 - segment1;
		return segment1 + glm::clamp(glm::dot(point - segment1, s), 0.f, glm::dot(s, s)) / glm::dot(s, s) * s;
	}
	inline std::pair<glm::vec3, glm::vec3> getSegmentsClosestSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
	{
		glm::vec3 s1 = segment1b - segment1a;
		glm::vec3 s2 = segment2b - segment2a;
		glm::vec3 n = glm::cross(s1, s2);
		glm::vec3 n2 = glm::cross(segment1b - segment1a, segment2a - segment1a);

		if (n == glm::vec3(0.f))	// parallel or one segment is a point
		{
			if (s1 == glm::vec3(0.f) && s2 == glm::vec3(0.f))
				return std::pair<glm::vec3, glm::vec3>(segment1a, segment2a);
			else if (s1 == glm::vec3(0.f))
				return std::pair<glm::vec3, glm::vec3>(segment1a, getSegmentClosestPoint(segment2a, segment2b, segment1a));
			else if (s2 == glm::vec3(0.f))
				return std::pair<glm::vec3, glm::vec3>(segment2a, getSegmentClosestPoint(segment1a, segment1b, segment2a));
			else // segment are parallel
			{
				auto d1 = std::pair<glm::vec3, glm::vec3>(segment1a, getSegmentClosestPoint(segment2a, segment2b, segment1a));
				auto d2 = std::pair<glm::vec3, glm::vec3>(segment1b, getSegmentClosestPoint(segment2a, segment2b, segment1b));
				auto d3 = std::pair<glm::vec3, glm::vec3>(segment2a, getSegmentClosestPoint(segment1a, segment1b, segment2a));
				auto d4 = std::pair<glm::vec3, glm::vec3>(segment2b, getSegmentClosestPoint(segment1a, segment1b, segment2b));
				
				float f1 = glm::dot(d1.first - d1.second, d1.first - d1.second);
				float f2 = glm::dot(d2.first - d2.second, d2.first - d2.second);
				float f3 = glm::dot(d3.first - d3.second, d3.first - d3.second);
				float f4 = glm::dot(d4.first - d4.second, d4.first - d4.second);

				if (f1 < f2 && f1 < f3 && f1 < f4) return d1;
				else if (f2 < f1 && f2 < f3 && f2 < f4) return d2;
				else if (f3 < f1 && f3 < f2 && f3 < f4) return d3;
				else return d4;
			}
		}
		else if (glm::dot(n2, segment2b) == 0.f) // coplanar segments
		{
			// https://www.lucidarme.me/intersection-of-segments/ 

			glm::vec3 ss = segment2a - segment1a;
			float t1 = glm::dot(glm::cross(ss, s2), n) / glm::dot(n, n);
			float t2 = glm::dot(glm::cross(ss, s1), n) / glm::dot(n, n);

			return std::pair<glm::vec3, glm::vec3>(segment1a + s1*glm::clamp(t1, 0.f, 1.f), segment2a + s2*glm::clamp(t2, 0.f, 1.f));
		}
		else //	skew segment
		{
			glm::vec3 u1 = glm::normalize(s1);
			glm::vec3 u2 = glm::normalize(s2);
			n = glm::normalize(n);
			float t1 = -glm::determinant(glm::mat3(segment1a - segment2a, u2, n))/* / glm::dot(n, n)*/;
			float t2 = -glm::determinant(glm::mat3(segment1a - segment2a, u1, n))/* / glm::dot(n, n)*/;
			return std::pair<glm::vec3, glm::vec3>(segment2a + u2*t2, segment1a + u1*t1);
		}
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