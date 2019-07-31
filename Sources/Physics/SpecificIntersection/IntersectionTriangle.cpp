#include "IntersectionTriangle.h"

#include "Physics/SpecificCollision/CollisionUtils.h"
#include "Physics/SpecificCollision/CollisionSegment.h"
#include "Physics/SpecificCollision/CollisionTriangle.h"

#include "IntersectionSegment.h"
#include "IntersectionPoint.h"

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
	/*
	Contact result;
	if (capsule1 == capsule2) 
	{
		result = intersect_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius);
		result.normal1 = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));
		result.normal2 = -result.normal1;
	}
	else
	{
		if (Collision::collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius))
		{
			result = intersect_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius);
			result.normal1 = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));

			glm::vec3 s = capsule2 - capsule1;
			float f = glm::dot(s, result.contact2 - capsule1) / glm::dot(s, s);
			if (f >= 0.f && f <= 1.f)
			{
				glm::vec3 c = getSegmentClosestPoint(capsule1, capsule2, result.contact2);
				result.normal2 = glm::normalize(result.contact2 - c);
				result.contact2 = c + capsuleRadius * result.normal2;
			}
		}
		else if (Collision::collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule2, capsuleRadius))
		{
			result = intersect_TrianglevsSphere(triangle1, triangle2, triangle3, capsule2, capsuleRadius);
			result.normal1 = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));
			
			glm::vec3 s = capsule2 - capsule1;
			float f = glm::dot(s, result.contact2 - capsule1) / glm::dot(s, s);
			if (f >= 0.f && f <= 1.f)
			{
				glm::vec3 c = getSegmentClosestPoint(capsule1, capsule2, result.contact2);
				result.normal2 = glm::normalize(result.contact2 - c);
				result.contact2 = c + capsuleRadius * result.normal2;
			}
		}
		else if (Collision::collide_SegmentvsCapsule(triangle1, triangle2, capsule1, capsule2, capsuleRadius))
		{
			result = intersect_SegmentvsCapsule(triangle1, triangle2, capsule1, capsule2, capsuleRadius);
			result.normal1 = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));
		}
		else if (Collision::collide_SegmentvsCapsule(triangle2, triangle3, capsule1, capsule2, capsuleRadius))
		{
			result = intersect_SegmentvsCapsule(triangle2, triangle3, capsule1, capsule2, capsuleRadius);
			result.normal1 = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));
		}
		else if (Collision::collide_SegmentvsCapsule(triangle3, triangle1, capsule1, capsule2, capsuleRadius))
		{
			result = intersect_SegmentvsCapsule(triangle3, triangle1, capsule1, capsule2, capsuleRadius);
			result.normal1 = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));
		}
	}
	return result;
	*/
}
