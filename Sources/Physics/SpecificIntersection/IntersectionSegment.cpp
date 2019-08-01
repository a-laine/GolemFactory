#include "IntersectionSegment.h"

#include "IntersectionPoint.h"
#include "Physics/SpecificCollision/CollisionUtils.h"
#include "Physics/SpecificCollision/CollisionSegment.h"

#include <iostream>

#include <glm/gtx/norm.hpp>


Intersection::Contact Intersection::intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	glm::vec3 s1 = segment1b - segment1a;
	glm::vec3 s2 = segment2b - segment2a;
	glm::vec3 n = glm::cross(s1, s2);

	if (n == glm::vec3(0.f))	// parallel or one segment is a point
	{
		if (s1 == glm::vec3(0.f))
			return intersect_PointvsSegment(segment1a, segment2a, segment2b);
		else if (s2 == glm::vec3(0.f))
			return intersect_PointvsSegment(segment2a, segment1a, segment1b);
		else // segment are parallel
		{
			// compute closest point of each corner of the paralellogram
			glm::vec3 p1 = getSegmentClosestPoint(segment1a, segment1b, segment2a);
			glm::vec3 p2 = getSegmentClosestPoint(segment1a, segment1b, segment2b);
			glm::vec3 p3 = getSegmentClosestPoint(segment2a, segment2b, segment1a);
			glm::vec3 p4 = getSegmentClosestPoint(segment2a, segment2b, segment1b);

			float d1 = glm::length2(p1 - segment2a);
			float d2 = glm::length2(p2 - segment2b);
			float d3 = glm::length2(p3 - segment1a);
			float d4 = glm::length2(p4 - segment1b);

			// search for the minimal distance of each pair
			Contact contact;
			if (d1 < d2 && d1 < d3 && d1 < d4)
			{
				contact.contactPointA = p1;
				contact.contactPointB = segment2a;
			}
			if (d2 < d1 && d2 < d3 && d2 < d4)
			{
				contact.contactPointA = p2;
				contact.contactPointB = segment2b;
			}
			if (d3 < d2 && d3 < d1 && d3 < d4)
			{
				contact.contactPointB = p3;
				contact.contactPointA = segment1a;
			}
			else
			{
				contact.contactPointB = p4;
				contact.contactPointA = segment1b;
			}

			// finish computing result
			contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
			contact.normalB = -contact.normalA;
			return contact;
		}
	}
	else
	{
		// compute closest segment
		glm::vec3 u1 = glm::normalize(s1);
		glm::vec3 u2 = glm::normalize(s2);
		n = glm::normalize(n);
		float t1 = -glm::determinant(glm::mat3(segment1a - segment2a, u2, n)) / glm::dot(n, n);
		float t2 = -glm::determinant(glm::mat3(segment1a - segment2a, u1, n)) / glm::dot(n, n);

		// compute result
		Contact contact;
		contact.contactPointA = segment1a + u1 * t1;
		contact.contactPointB = segment2a + u2 * t2;
		contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
		contact.normalB = -contact.normalA;
		return contact;
	}
}
Intersection::Contact Intersection::intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	//	begin and eliminate special cases
	glm::vec3 v1 = triangle2 - triangle1;
	glm::vec3 v2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(v1, v2);

	if (n == glm::vec3(0.f)) // flat triangle
	{
		glm::vec3 v3 = triangle3 - triangle2;
		float d1 = glm::dot(v1, v1);
		float d2 = glm::dot(v2, v2);
		float d3 = glm::dot(v3, v3);

		if (d1 >= d2 && d1 >= d3) return intersect_SegmentvsSegment(segment1, segment2, triangle1, triangle2);
		else if (d2 >= d1 && d2 >= d3) return intersect_SegmentvsSegment(segment1, segment2, triangle1, triangle3);
		else return intersect_SegmentvsSegment(segment1, segment2, triangle3, triangle2);
	}
	glm::vec3 intersection;
	if (Collision::collide_SegmentvsTriangle(segment1, segment2, triangle1, triangle2, triangle3, intersection))
	{
		Contact contact;
		contact.contactPointA = intersection;
		contact.contactPointB = intersection;
		contact.normalB = n;
		if (glm::cross(n, segment2 - segment1) != glm::vec3(0.f))
			contact.normalA = -n;
		else
			contact.normalB = glm::cross(n, glm::cross(segment2 - segment1, n));
		return contact;
	}

	// compute segment extremity projection on triangle plane
	n = glm::normalize(n);
	glm::vec3 p1 = segment1 - triangle1 - n * glm::dot(segment1 - triangle1, n);
	glm::vec2 b1 = getBarycentricCoordinates(v1, v2, p1);
	glm::vec3 pair1 = segment1;
	glm::vec3 p2 = segment2 - triangle1 - n * glm::dot(segment2 - triangle1, n);
	glm::vec2 b2 = getBarycentricCoordinates(v1, v2, p2);
	glm::vec3 pair2 = segment2;
	float dp1 = std::numeric_limits<float>::max();
	float dp2 = std::numeric_limits<float>::max();
	
	// check if projection is on triangle and get the closest one in position 1
	if (b1.x >= 0.f || b1.y >= 0.f || b1.x + b1.y <= 1.f)
		dp1 = std::abs(glm::dot(segment1 - triangle1, n));
	if (b2.x >= 0.f || b2.y >= 0.f || b2.x + b2.y <= 1.f)
		dp1 = std::abs(glm::dot(segment2 - triangle1, n));
	if (dp2 < dp1)
	{
		dp1 = dp2;
		p1 = p2;
		pair1 = pair2;
	}
	if(dp1 != std::numeric_limits<float>::max())
		dp1 *= dp1;
		
	// compute closest segment of each edge with segment
	std::pair<glm::vec3, glm::vec3> s1 = getSegmentsClosestSegment(segment1, segment2, triangle1, triangle2);
	std::pair<glm::vec3, glm::vec3> s2 = getSegmentsClosestSegment(segment1, segment2, triangle1, triangle3);
	std::pair<glm::vec3, glm::vec3> s3 = getSegmentsClosestSegment(segment1, segment2, triangle2, triangle3);
	float d1 = glm::length2(s1.first - s1.second);
	float d2 = glm::length2(s2.first - s2.second);
	float d3 = glm::length2(s3.first - s3.second);

	if (dp1 < d1 && dp1 < d2 && dp1 < d3)
	{

	}
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