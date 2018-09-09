#include "CollisionSphere.h"
#include "CollisionUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

//	Specialized functions : sphere
bool Collision::collide_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius)
{
	return glm::length(sphere2Center - sphere1Center) <= sphere1Radius + sphere2Radius;
}
bool Collision::collide_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return glm::length(sphereCenter - getSegmentClosestPoint(capsule1, capsule2, sphereCenter)) <= sphereRadius + capsuleRadius;
}
//