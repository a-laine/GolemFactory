#pragma once

#include "../Shape.h"

namespace Intersection
{
	//	Specialized functions : Axis Aligned Box vs all other (except previous shape)
	Result intersect_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
	Result intersect_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius);
	Result intersect_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
};
