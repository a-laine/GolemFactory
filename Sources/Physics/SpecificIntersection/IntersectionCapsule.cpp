#include "IntersectionCapsule.h"

#include "Physics/SpecificCollision/CollisionUtils.h"


Intersection::Contact Intersection::intersect_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	// init
	std::pair<glm::vec3, glm::vec3> s = getSegmentsClosestSegment(capsule1a, capsule1b, capsule2a, capsule2b);
	Contact contact;
	contact.normalA = glm::normalize(s.second - s.first);
	contact.normalB = -contact.normalA;
	contact.contactPointA = s.first + contact.normalA * capsule1Radius;
	contact.contactPointB = s.second + contact.normalB * capsule2Radius;
	return contact;
}