#pragma once

#include <algorithm>

#include "Physics/BoundingVolume.h"

namespace Collision
{
	//	Specialized functions : point vs all other
	bool collide_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2);
	bool collide_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2);
	bool collide_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
	bool collide_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
	bool collide_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax);
	bool collide_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius);
	bool collide_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	bool collide_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase);
};


