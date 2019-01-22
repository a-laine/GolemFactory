#include "IntersectionPoint.h"



Intersection::Contact Intersection::intersect_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return Intersection::Contact();
	/*
	Contact result;
	result.contact1 = point;
	result.contact2 = sphereCenter + sphereRadius * (sphereCenter - point);
	result.normal2 = glm::normalize(sphereCenter - point);
	result.normal1 = -result.normal2;
	return result;
	*/
}
Intersection::Contact Intersection::intersect_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return Intersection::Contact();
	/*if (capsule2 == capsule1) return intersect_PointvsSphere(point, capsule1, capsuleRadius);
	else
	{
		Contact result;
		glm::vec3 u = glm::normalize(capsule2 - capsule1);
		float t = glm::clamp(glm::dot(u, point - capsule1), 0.f, glm::length(capsule2 - capsule1));

		result.contact1 = point;
		result.contact2 = capsule1 + t * u;
		result.normal2 = glm::normalize(result.contact1 - result.contact2);
		result.normal1 = -result.normal2;
		result.contact2 += capsuleRadius * result.normal2;
		return result;
	}*/
}