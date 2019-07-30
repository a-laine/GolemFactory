#include "IntersectionPoint.h"
#include "Physics/SpecificCollision/CollisionUtils.h"


Intersection::Contact Intersection::intersect_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2)
{
	glm::vec3 n = glm::normalize(point2 - point1);
	return Intersection::Contact(point1, point2, n, -n);
}
Intersection::Contact Intersection::intersect_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2)
{
	Contact contact;
	contact.contactPointA = point;
	contact.contactPointB = getSegmentClosestPoint(segment1, segment2, point);
	if (contact.contactPointB == segment1)
		contact.normalB = glm::normalize(point - segment1);
	else if (contact.contactPointB == segment2)
		contact.normalB = glm::normalize(point - segment2);
	else
		contact.normalB = glm::normalize(point - contact.contactPointB);
	contact.normalA = -contact.normalB;
	return contact;
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
	Contact contact;
	contact.normalB = glm::normalize(sphereCenter - point);
	contact.normalA = -contact.normalB;
	contact.contactPointA = point;
	contact.contactPointB = sphereCenter + sphereRadius * contact.normalA;
	return contact;
}
Intersection::Contact Intersection::intersect_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact;
	contact.contactPointA = point;
	contact.contactPointB = getSegmentClosestPoint(capsule1, capsule2, point);
	if (contact.contactPointB == capsule1)
	{
		contact.normalB = glm::normalize(point - capsule1);
		contact.contactPointB = capsule1 + capsuleRadius * contact.normalB;
	}
	else if (contact.contactPointB == capsule2)
	{
		contact.normalB = glm::normalize(point - capsule2);
		contact.contactPointB = capsule2 + capsuleRadius * contact.normalB;
	}
	else
	{
		contact.normalB = glm::normalize(point - contact.contactPointB);
		contact.contactPointB = contact.contactPointB + capsuleRadius * contact.normalB;
	}
	contact.normalA = -contact.normalB;
	return contact;
}
Intersection::Contact Intersection::intersect_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase)
{
	return Intersection::Contact();
	/*Intersection::Contact contact;
	bool outside = false;
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * glm::vec4(point, 1.f));
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		float d = 
		if (glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]) >= 0)
		{
			outside = true;

		}
	}
	if(outside)
	{
		contact
	}
	else
	{
		contact.contactPointA = point;
		contact.contactPointB = point;
	}
	return contact;*/
}