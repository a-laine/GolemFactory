#include "IntersectionSegment.h"

#include "IntersectionPoint.h"

#include <iostream>



Intersection::Result Intersection::intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_SegmentvsOrientedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_SegmentvsAxisAlignedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return Intersection::Result();
}
Intersection::Result Intersection::intersect_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (segment2 == segment1) return intersect_PointvsSphere(segment1, sphereCenter, sphereRadius);

	Result result;
	glm::vec3 s = segment2 - segment1;
	glm::vec3 u = glm::normalize(s);
	glm::vec3 u2 = sphereCenter - segment1;
	result.normal2 = glm::normalize(u2 - glm::dot(u, u2) * u);
	result.normal1 = -result.normal2;

	float t = glm::clamp(glm::dot(u, u2), 0.f, glm::length(s));
	result.contact1 = segment1 + t * u;
	result.contact2 = sphereCenter + sphereRadius * result.normal2;
	return result;
}
Intersection::Result Intersection::intersect_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	glm::vec3 s1 = segment2 - segment1;
	glm::vec3 s2 = capsule2 - capsule1;
	glm::vec3 n = glm::cross(s1, s2);

	if (n == glm::vec3(0.f))	// parallel or one segment is a point
	{
		if (s1 == glm::vec3(0.f))
			return intersect_PointvsCapsule(segment1, capsule1, capsule2, capsuleRadius);
		else if (s2 == glm::vec3(0.f))
			return intersect_SegmentvsSphere(segment1, segment2, capsule1, capsuleRadius);
		else // segment are parallel
		{
			Result result;
			result.contact1 = segment1;

			glm::vec3 u1 = glm::normalize(s1);
			glm::vec3 u3 = segment1 - capsule1;
			glm::vec3 d = u3 - u1 * std::abs(glm::dot(u3, u1));
			std::cout << "toto" << std::endl;

			result.normal2 = glm::normalize(d);
			result.normal1 = -result.normal1;
			result.contact2 = capsule1 + capsuleRadius * result.normal2;
			return result;
		}
	}
	else
	{
		Result result;
		glm::vec3 u1 = glm::normalize(s1);
		glm::vec3 u2 = glm::normalize(s2);
		n = glm::normalize(n);
		float t1 = -glm::determinant(glm::mat3(segment1 - capsule1, u2, n)) / glm::dot(n, n);
		float t2 = -glm::determinant(glm::mat3(segment1 - capsule1, u1, n)) / glm::dot(n, n);

		t1 = glm::clamp(t1, 0.f, glm::length(s1));
		t2 = glm::clamp(t2, 0.f, glm::length(s2));

		result.contact2 = capsule1 + u2*t2;
		result.contact1 = segment1 + u1*t1;
		result.normal2 = glm::normalize(result.contact2 - result.contact1);
		result.normal1 = -result.normal1;
		result.contact2 += capsuleRadius * result.normal2;
		return result;
	}
}