#pragma once

#include "Physics/BoundingVolume.h"
#include "IntersectionContact.h"

namespace Intersection
{
	//	Specialized functions : Sphere vs all other (except previous Shape)
	Contact intersect_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius);
	Contact intersect_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	Contact intersect_SpherevsHull(const glm::vec3& sphereCenter, const float& sphereRadius, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase);
};
