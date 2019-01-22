#include "IntersectionSegment.h"

#include "IntersectionPoint.h"
#include "Physics/SpecificCollision/CollisionUtils.h"

#include <iostream>



Intersection::Contact Intersection::intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_SegmentvsOrientedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_SegmentvsAxisAlignedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return Intersection::Contact();
	/*
	if (segment2 == segment1) return intersect_PointvsSphere(segment1, sphereCenter, sphereRadius);

	Contact result;
	glm::vec3 s = segment2 - segment1;
	glm::vec3 u = glm::normalize(s);
	glm::vec3 u2 = sphereCenter - segment1;
	result.normal2 = glm::normalize(u2 - glm::dot(u, u2) * u);
	result.normal1 = -result.normal2;

	float t = glm::clamp(glm::dot(u, u2), 0.f, glm::length(s));
	result.contact1 = segment1 + t * u;
	result.contact2 = sphereCenter + sphereRadius * result.normal2;
	return result;
	*/
}
Intersection::Contact Intersection::intersect_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return Intersection::Contact();
	/*
	glm::vec3 s1 = segment2 - segment1;
	glm::vec3 s2 = capsule2 - capsule1;
	if (s1 == glm::vec3(0.f))
		return intersect_PointvsCapsule(segment1, capsule1, capsule2, capsuleRadius);
	else if (s2 == glm::vec3(0.f))
		return intersect_SegmentvsSphere(segment1, segment2, capsule1, capsuleRadius);

	Contact result;
	std::pair<glm::vec3, glm::vec3> closest = getSegmentsClosestSegment(segment1, segment2, capsule1, capsule2);
	result.contact1 = closest.first;
	result.normal2 = glm::normalize(closest.first - closest.second);
	result.normal1 = -result.normal2;
	result.contact2 = closest.second + capsuleRadius * result.normal2;

	return result;
	*/
}