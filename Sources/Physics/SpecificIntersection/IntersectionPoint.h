#pragma once

#include "Physics/BoundingVolume.h"
#include "IntersectionContact.h"

namespace Intersection
{
	//	Specialized functions : point vs all other
	Contact intersect_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2);
	Contact intersect_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2);
	Contact intersect_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
	Contact intersect_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, glm::vec3* e1, glm::vec3* e2);
	Contact intersect_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
	Contact intersect_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax);
	Contact intersect_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius);
	Contact intersect_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	Contact intersect_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase);
}