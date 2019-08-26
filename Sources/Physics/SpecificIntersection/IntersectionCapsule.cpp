#include "IntersectionCapsule.h"

#include "Physics/SpecificCollision/CollisionUtils.h"


Intersection::Contact Intersection::intersect_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	// init
	Contact contact;
	std::pair<glm::vec3, glm::vec3> p = getSegmentsClosestSegment(capsule1a, capsule1b, capsule2a, capsule2b);
	contact.contactPointA = p.first;// + contact.normalA * capsule1Radius;
	contact.contactPointB = p.second;// + contact.normalB * capsule2Radius;
	contact.normalA = glm::normalize(contact.contactPointB - contact.contactPointA);
	contact.normalB = -contact.normalA;
	return contact;
}