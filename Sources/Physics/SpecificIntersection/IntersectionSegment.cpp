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
			else if (d2 < d1 && d2 < d3 && d2 < d4)
			{
				contact.contactPointA = p2;
				contact.contactPointB = segment2b;
			}
			else if (d3 < d2 && d3 < d1 && d3 < d4)
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

	// eliminate collision case
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
		
	// compute all segments of each edge with test segment
	std::pair<glm::vec3, glm::vec3> s1 = getSegmentsClosestSegment(segment1, segment2, triangle1, triangle2);
	std::pair<glm::vec3, glm::vec3> s2 = getSegmentsClosestSegment(segment1, segment2, triangle1, triangle3);
	std::pair<glm::vec3, glm::vec3> s3 = getSegmentsClosestSegment(segment1, segment2, triangle2, triangle3);
	float d1 = glm::length2(s1.first - s1.second);
	float d2 = glm::length2(s2.first - s2.second);
	float d3 = glm::length2(s3.first - s3.second);

	// choose closest point set
	Contact contact;
	if (dp1 < d1 && dp1 < d2 && dp1 < d3)
	{
		contact.contactPointA = pair1;
		contact.contactPointB = p1;
	}
	else if (d1 < d2 && d1 < d3)
	{
		contact.contactPointA = s1.first;
		contact.contactPointB = s1.second;
	}
	else if (d2 < d1 && d2 < d3)
	{
		contact.contactPointA = s2.first;
		contact.contactPointB = s2.second;
	}
	else
	{
		contact.contactPointA = s3.first;
		contact.contactPointB = s3.second;
	}

	// end and return
	contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
	contact.normalB = -contact.normalA;
	return contact;
}
Intersection::Contact Intersection::intersect_SegmentvsOrientedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	// init
	glm::vec3 s1 = glm::vec3(glm::inverse(boxTranform) * glm::vec4(segment1, 1.f));
	glm::vec3 s2 = glm::vec3(glm::inverse(boxTranform) * glm::vec4(segment2, 1.f));
	Contact contact = intersect_SegmentvsAxisAlignedBox(s1, s2, boxMin, boxMax);

	// end
	contact.contactPointA = glm::vec3(boxTranform * glm::vec4(contact.contactPointA, 1.f));
	contact.contactPointB = glm::vec3(boxTranform * glm::vec4(contact.contactPointB, 1.f));
	contact.normalA = glm::vec3(boxTranform * glm::vec4(contact.normalA, 0.f));
	contact.normalB = glm::vec3(boxTranform * glm::vec4(contact.normalB, 0.f));
	return contact;
}
Intersection::Contact Intersection::intersect_SegmentvsAxisAlignedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	if (segment2 == segment1) return intersect_PointvsAxisAlignedBox(segment1, boxMin, boxMax);

	if (Collision::collide_SegmentvsAxisAlignedBox(segment1, segment2, boxMin, boxMax))
	{
		glm::vec3 bcenter = 0.5f * (boxMax + boxMin);
		glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
		
		Contact contact;
		contact.contactPointA = getSegmentClosestPoint(segment1, segment2, bcenter);
		glm::vec3 p = contact.contactPointA - bcenter;
		glm::vec3 n = glm::vec3(0.f);

		// inside point is closer to an x face
		if (std::abs(p.x) > std::abs(p.y) && std::abs(p.x) > std::abs(p.z))
		{
			if (p.x > 0)
			{
				n = glm::vec3(1, 0, 0);
				p.x = bsize.x;
			}
			else
			{
				n = glm::vec3(-1, 0, 0);
				p.x = -bsize.x;
			}
		}

		// inside point is closer to a y face
		else if (std::abs(p.y) > std::abs(p.x) && std::abs(p.y) > std::abs(p.z))
		{
			if (p.y > 0)
			{
				n = glm::vec3(0, 1, 0);
				p.y = bsize.y;
			}
			else
			{
				n = glm::vec3(0, -1, 0);
				p.y = -bsize.y;
			}
		}

		// inside point is closer to a z face
		else
		{
			if (p.z > 0)
			{
				n = glm::vec3(0, 0, 1);
				p.z = bsize.z;
			}
			else
			{
				n = glm::vec3(0, 0, -1);
				p.z = -bsize.z;
			}
		}

		// end and return
		contact.contactPointB = p + bcenter;
		contact.normalB = n;
		contact.normalA = -glm::cross(segment2 - segment1, glm::cross(n, segment2 - segment1));
		return contact;
	}
	else
	{
		// add closest point on ABB to both segment extremities
		std::vector<std::pair<glm::vec3, glm::vec3>> candidates;
		Contact c = intersect_PointvsAxisAlignedBox(segment1, boxMin, boxMax);
		candidates.push_back({ c.contactPointA, c.contactPointB });
		c = intersect_PointvsAxisAlignedBox(segment2, boxMin, boxMax);
		candidates.push_back({ c.contactPointA, c.contactPointB });

		// adding all cube edges to candidates
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, boxMin, glm::vec3(boxMin.x, boxMin.y, boxMax.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, boxMin, glm::vec3(boxMin.x, boxMax.y, boxMin.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, boxMin, glm::vec3(boxMax.x, boxMin.y, boxMin.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, boxMax, glm::vec3(boxMax.x, boxMax.y, boxMin.x)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, boxMax, glm::vec3(boxMax.x, boxMin.y, boxMax.x)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, boxMax, glm::vec3(boxMin.x, boxMax.y, boxMax.x)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, glm::vec3(boxMin.x, boxMax.y, boxMax.z), glm::vec3(boxMin.x, boxMin.y, boxMax.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, glm::vec3(boxMin.x, boxMax.y, boxMax.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, glm::vec3(boxMax.x, boxMax.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, glm::vec3(boxMax.x, boxMax.y, boxMin.z), glm::vec3(boxMax.x, boxMin.y, boxMin.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMin.x, boxMin.y, boxMax.z)));
		candidates.push_back(getSegmentsClosestSegment(segment1, segment2, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMin.y, boxMin.z)));
		
		// sorting functions
		auto compare = [](std::pair<glm::vec3, glm::vec3> s1, std::pair<glm::vec3, glm::vec3> s2) {
			return glm::length2(s1.first - s1.second) < glm::length2(s2.first - s2.second);
		};
		std::sort(candidates.begin(), candidates.end(), compare);

		// end and return
		Contact contact;
		contact.contactPointA = candidates.front().first;
		contact.contactPointB = candidates.front().second;
		contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
		contact.normalB = -contact.normalA;
		return contact;
	}
}
Intersection::Contact Intersection::intersect_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (segment2 == segment1) return intersect_PointvsSphere(segment1, sphereCenter, sphereRadius);

	Contact contact;
	contact.contactPointA = getSegmentClosestPoint(segment1, segment2, sphereCenter);
	contact.contactPointB = sphereCenter;
	contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
	contact.normalB = -contact.normalA;
	return contact;
}
Intersection::Contact Intersection::intersect_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact = intersect_SegmentvsSegment(segment1, segment2, capsule1, capsule2);
	contact.contactPointB = contact.contactPointB + capsuleRadius * contact.normalB;
	return contact;
}