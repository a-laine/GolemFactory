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
	//	Utils
	//glm::vec3 getSegmentClosestPoint(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& point);
	vec4f getSegmentClosestPoint(const vec4f& segment1, const vec4f& segment2, const vec4f& point);
	std::pair<vec4f, vec4f> getSegmentsClosestSegment(const vec4f& segment1a, const vec4f& segment1b, const vec4f& segment2a, const vec4f& segment2b);
	//glm::vec3 getTriangleClosestPoint(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& point);
	//glm::vec2 getBarycentricCoordinates(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& point, const bool& clamped = false);
	vec2f getBarycentricCoordinates(const vec4f& v1, const vec4f& v2, const vec4f& point, const bool& clamped = false);
	//
};
