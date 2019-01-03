#pragma once

#include "../Shape.h"

namespace Intersection
{
	//	Specialized functions : Segment vs all other (except previous shape)
	Result intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b);
	Result intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
	Result intersect_SegmentvsOrientedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
	Result intersect_SegmentvsAxisAlignedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& boxMin, const glm::vec3& boxMax);
	Result intersect_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius);
	Result intersect_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
}