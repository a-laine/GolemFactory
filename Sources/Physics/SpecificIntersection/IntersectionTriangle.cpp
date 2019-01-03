#include "IntersectionTriangle.h"

#include "../SpecificCollision/CollisionUtils.h"
#include "../SpecificCollision/CollisionSegment.h"
#include "../SpecificCollision/CollisionTriangle.h"

#include "IntersectionSegment.h"

Intersection::Result Intersection::intersect_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	//	initialize
	Result result;
	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	result.normal1 = glm::normalize(glm::cross(u1, u2));

	//	getting closest point in triangle from barycentric coordinates
	glm::vec2 bary = getBarycentricCoordinates(u1, u2, (sphereCenter - triangle1) - glm::dot(sphereCenter - triangle1, result.normal1) * result.normal1);
	if (bary.x < 0.f) bary.x = 0.f;
	if (bary.y < 0.f) bary.y = 0.f;
	if (bary.x + bary.y > 1.f) bary /= (bary.x + bary.y);

	result.contact1 = triangle1 + bary.x*u1 + bary.y*u2;
	result.normal2 = glm::normalize(result.contact1 - sphereCenter);
	result.contact2 = sphereCenter + sphereRadius * result.normal2;
	return result;
}
Intersection::Result Intersection::intersect_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (capsule1 == capsule2) return intersect_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius);
	if (Collision::collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius))
		return intersect_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius);
	if (Collision::collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule2, capsuleRadius))
		return intersect_TrianglevsSphere(triangle1, triangle2, triangle3, capsule2, capsuleRadius);

	if (Collision::collide_SegmentvsCapsule(triangle1, triangle2, capsule1, capsule2, capsuleRadius))
		return intersect_SegmentvsCapsule(triangle1, triangle2, capsule1, capsule2, capsuleRadius);
	if (Collision::collide_SegmentvsCapsule(triangle2, triangle3, capsule1, capsule2, capsuleRadius))
		return intersect_SegmentvsCapsule(triangle2, triangle3, capsule1, capsule2, capsuleRadius);
	if (Collision::collide_SegmentvsCapsule(triangle3, triangle1, capsule1, capsule2, capsuleRadius))
		return intersect_SegmentvsCapsule(triangle3, triangle1, capsule1, capsule2, capsuleRadius);

	return Intersection::Result();
}
