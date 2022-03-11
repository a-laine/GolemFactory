#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>

#define COLLISION_EPSILON 0.0001f

namespace CollisionUtils
{
	//	Utils
	glm::vec3 getSegmentClosestPoint(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& point);
	glm::vec4 getSegmentClosestPoint(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& point);
	std::pair<glm::vec4, glm::vec4> getSegmentsClosestSegment(const glm::vec4& segment1a, const glm::vec4& segment1b, const glm::vec4& segment2a, const glm::vec4& segment2b);
	//glm::vec3 getTriangleClosestPoint(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& point);
	glm::vec2 getBarycentricCoordinates(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& point, const bool& clamped = false);
	glm::vec2 getBarycentricCoordinates(const glm::vec4& v1, const glm::vec4& v2, const glm::vec4& point, const bool& clamped = false);
	//
};
