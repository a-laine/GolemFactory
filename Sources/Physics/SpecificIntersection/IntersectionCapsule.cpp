#include "IntersectionCapsule.h"

#include "IntersectionSphere.h"
#include <Physics/SpecificCollision/CollisionUtils.h>


/*Intersection::Contact Intersection::intersect_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	if (capsule1a == capsule1b) return intersect_SpherevsCapsule(capsule1a, capsule1Radius, capsule2a, capsule2b, capsule2Radius);
	else if (capsule2a == capsule2b) return intersect_SpherevsCapsule(capsule2a, capsule2Radius, capsule1a, capsule1b, capsule1Radius).swap();

	Contact contact;
	std::pair<glm::vec3, glm::vec3> p = CollisionUtils::getSegmentsClosestSegment(capsule1a, capsule1b, capsule2a, capsule2b);
	contact.normalA = glm::normalize(p.second - p.first);
	contact.normalB = -contact.normalA;
	contact.contactPointA = p.first + contact.normalA * capsule1Radius;
	contact.contactPointB = p.second + contact.normalB * capsule2Radius;
	return contact;
}*/