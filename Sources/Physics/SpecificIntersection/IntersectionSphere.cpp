#include "IntersectionSphere.h"
#include "Physics/SpecificCollision/CollisionUtils.h"
#include "IntersectionPoint.h"

Intersection::Contact Intersection::intersect_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius)
{
	glm::vec3 n = glm::normalize(sphere2Center - sphere1Center);
	return Intersection::Contact(sphere1Center + sphere1Radius * n, sphere2Center - sphere2Radius * n, n, -n);
}
Intersection::Contact Intersection::intersect_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact;
	contact.contactPointB = getSegmentClosestPoint(capsule1, capsule2, sphereCenter);
	if (contact.contactPointB == capsule1)
	{
		contact.normalB = glm::normalize(sphereCenter - capsule1);
		contact.contactPointB = capsule1 + capsuleRadius * contact.normalB;
	}
	else if (contact.contactPointB == capsule2)
	{
		contact.normalB = glm::normalize(sphereCenter - capsule2);
		contact.contactPointB = capsule2 + capsuleRadius * contact.normalB;
	}
	else
	{
		contact.normalB = glm::normalize(sphereCenter - contact.contactPointB);
		contact.contactPointB = contact.contactPointB + capsuleRadius * contact.normalB;
	}
	contact.normalA = -contact.normalB;
	contact.contactPointA = sphereCenter + sphereRadius * contact.normalA;
	return contact;
}
Intersection::Contact Intersection::intersect_SpherevsHull(const glm::vec3& sphereCenter, const float& sphereRadius, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase)
{
	Contact contact = intersect_PointvsHull(sphereCenter, hullPoints, hullNormals, hullFaces, hullBase);
	contact.contactPointA = contact.contactPointA + contact.normalA * sphereRadius;
	return contact;
}
