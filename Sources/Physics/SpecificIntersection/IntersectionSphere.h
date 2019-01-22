#pragma once

#include "Physics/BoundingVolume.h"
#include "IntersectionContact.h"

namespace Intersection
{
	//	Specialized functions : Sphere vs all other (except previous Shape)
	Contact intersect_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius);
	Contact intersect_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
};
