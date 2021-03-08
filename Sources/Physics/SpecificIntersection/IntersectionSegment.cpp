#include "IntersectionSegment.h"

#include "IntersectionPoint.h"
#include <Physics/SpecificCollision/CollisionUtils.h>
#include <Physics/SpecificCollision/CollisionSegment.h>

#include <iostream>

#include <glm/gtx/norm.hpp>


#include <Utiles/Debug.h>



Intersection::Contact Intersection::intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	if (segment1a == segment1b) return intersect_PointvsSegment(segment1a, segment2a, segment2b);
	else if (segment2a == segment2b) return intersect_PointvsSegment(segment2a, segment1a, segment2b).swap();

	Contact contact;
	std::pair<glm::vec3, glm::vec3> p = getSegmentsClosestSegment(segment1a, segment1b, segment2a, segment2b);
	contact.contactPointA = p.first;
	contact.contactPointB = p.second;
	contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
	contact.normalB = -contact.normalA;
	return contact;
}
Intersection::Contact Intersection::intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	//	begin and eliminate special cases
	if (segment1 == segment2) return intersect_PointvsTriangle(segment1, triangle1, triangle2, triangle3);
	glm::vec3 v1 = triangle2 - triangle1;
	glm::vec3 v2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(v1, v2);
	
	// flat triangle
	if (n == glm::vec3(0.f)) 
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
	if (Collision::collide_SegmentvsTriangle(segment1, segment2, triangle1, triangle2, triangle3, &intersection))
	{
		Contact contact;
		contact.contactPointA = intersection + triangle1;
		contact.contactPointB = contact.contactPointA;
		contact.normalB = glm::normalize(n);
		if (glm::cross(n, segment2 - segment1) != glm::vec3(0.f))
			contact.normalA = -contact.normalB;
		else
			contact.normalB = glm::normalize(glm::cross(n, glm::cross(segment2 - segment1, n)));
		return contact;
	}
			
	// compute all segments of each edge with test segment
	Contact c1 = intersect_PointvsTriangle(segment1, triangle1, triangle2, triangle3);
	Contact c2 = intersect_PointvsTriangle(segment2, triangle1, triangle2, triangle3);
	std::pair<glm::vec3, glm::vec3> s1 = getSegmentsClosestSegment(segment1, segment2, triangle1, triangle2);
	std::pair<glm::vec3, glm::vec3> s2 = getSegmentsClosestSegment(segment1, segment2, triangle1, triangle3);
	std::pair<glm::vec3, glm::vec3> s3 = getSegmentsClosestSegment(segment1, segment2, triangle2, triangle3);
	float d1 = glm::length2(s1.first - s1.second);
	float d2 = glm::length2(s2.first - s2.second);
	float d3 = glm::length2(s3.first - s3.second);
	float d4 = glm::length2(c1.contactPointA - c1.contactPointB);
	float d5 = glm::length2(c2.contactPointA - c2.contactPointB);

	// choose closest point set
	Contact contact;
	if (d4 <= d5 && d4 <= d1 && d4 <= d2 && d4 <= d3)
		return c1;
	else if (d5 <= d4 && d5 <= d1 && d5 <= d2 && d5 <= d3)
		return c2;
	else if (d1 <= d4 && d1 <= d5 && d1 <= d2 && d1 <= d3)
	{
		contact.contactPointA = s1.first;
		contact.contactPointB = s1.second;
	}
	else if (d2 <= d4 && d2 <= d5 && d2 <= d1 && d2 <= d3)
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
		std::vector<Contact> candidates;
		candidates.reserve(14);
		candidates.push_back(intersect_PointvsAxisAlignedBox(segment1, boxMin, boxMax));
		candidates.push_back(intersect_PointvsAxisAlignedBox(segment2, boxMin, boxMax));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, boxMin, glm::vec3(boxMin.x, boxMin.y, boxMax.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, boxMin, glm::vec3(boxMin.x, boxMax.y, boxMin.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, boxMin, glm::vec3(boxMax.x, boxMin.y, boxMin.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, boxMax, glm::vec3(boxMax.x, boxMax.y, boxMin.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, boxMax, glm::vec3(boxMax.x, boxMin.y, boxMax.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, boxMax, glm::vec3(boxMin.x, boxMax.y, boxMax.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, glm::vec3(boxMin.x, boxMax.y, boxMax.z), glm::vec3(boxMin.x, boxMin.y, boxMax.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, glm::vec3(boxMin.x, boxMax.y, boxMax.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, glm::vec3(boxMax.x, boxMax.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, glm::vec3(boxMax.x, boxMax.y, boxMin.z), glm::vec3(boxMax.x, boxMin.y, boxMin.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMin.x, boxMin.y, boxMax.z)));
		candidates.push_back(intersect_SegmentvsSegment(segment1, segment2, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMin.y, boxMin.z)));

		float dmin = std::numeric_limits<float>::max();
		int k = 0;
		for (unsigned int i = 0; i < candidates.size(); i++)
		{
			float d = glm::length2(candidates[i].contactPointA - candidates[i].contactPointB);
			if (d < dmin)
			{
				k = i;
				dmin = d;
			}
		}
		return candidates[k];
	}
}
Intersection::Contact Intersection::intersect_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (segment2 == segment1) return intersect_PointvsSphere(segment1, sphereCenter, sphereRadius);

	Contact contact;
	contact.contactPointA = getSegmentClosestPoint(segment1, segment2, sphereCenter);
	contact.normalA = glm::normalize(sphereCenter - contact.contactPointA);
	contact.normalB = -contact.normalA;
	contact.contactPointB = sphereCenter + sphereRadius * contact.normalB;
	return contact;
}
Intersection::Contact Intersection::intersect_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact = intersect_SegmentvsSegment(segment1, segment2, capsule1, capsule2);
	contact.contactPointB = contact.contactPointB + capsuleRadius * contact.normalB;
	return contact;
}