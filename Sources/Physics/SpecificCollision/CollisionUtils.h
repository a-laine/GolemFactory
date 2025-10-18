#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

/*#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>*/

#include "Math/TMath.h"

#define COLLISION_EPSILON 0.0001f

namespace CollisionUtils
{
	enum ClosestPointCategory
	{
		eInside,
		eSegment12,
		eSegment13,
		eSegment23,
		eVertex1,
		eVertex2,
		eVertex3
	};
	//	Utils
	vec4f getSegmentClosestPoint(const vec4f& segment1, const vec4f& segment2, const vec4f& point);
	std::pair<vec4f, vec4f> getSegmentsClosestSegment(const vec4f& segment1a, const vec4f& segment1b, const vec4f& segment2a, const vec4f& segment2b);
	vec4f getTriangleClosestPoint(const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, const vec4f& point, 
		ClosestPointCategory* optionnal0 = nullptr, vec3f* optionnal1 = nullptr);
	vec2f getBarycentricCoordinates(const vec4f& v1, const vec4f& v2, const vec4f& point, const bool& clamped = false);
	std::pair<vec4f, vec4f> getClosestPair(const vec4f& segment1, const vec4f& segment2, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3);
	//
};
