#include "IntersectionAxisAlignedBox.h"

Intersection::Contact Intersection::intersect_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return Intersection::Contact();
}

