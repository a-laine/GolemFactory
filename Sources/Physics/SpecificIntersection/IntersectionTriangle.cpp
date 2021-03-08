#include "IntersectionTriangle.h"
#include "IntersectionSegment.h"
#include "IntersectionPoint.h"

#include <Physics/SpecificCollision/CollisionUtils.h>
#include <Physics/SpecificCollision/CollisionSegment.h>
#include <Physics/SpecificCollision/CollisionTriangle.h>


Intersection::Contact Intersection::intersect_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	Contact contact = intersect_PointvsTriangle(sphereCenter, triangle1, triangle2, triangle3);
	contact.contactPointA = contact.contactPointA + sphereRadius * contact.normalA;
	return contact.swap();
}
Intersection::Contact Intersection::intersect_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return Intersection::Contact();
}
