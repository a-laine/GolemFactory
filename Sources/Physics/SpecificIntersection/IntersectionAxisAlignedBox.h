#pragma once

#include "Physics/BoundingVolume.h"
#include "IntersectionContact.h"

namespace Intersection
{
	//	Specialized functions : Axis Aligned Box vs all other (except previous Shape)
	Contact intersect_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
	Contact intersect_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius);
	Contact intersect_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
};
