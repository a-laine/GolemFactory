#include "IntersectionContact.h"

Intersection::Contact::Contact() : contactPointA(0.f), contactPointB(0.f), normalA(0.f), normalB(0.f)
{}
Intersection::Contact::Contact(const glm::vec3& A, const glm::vec3& B, const glm::vec3& nA, const glm::vec3& nB) : contactPointA(A), contactPointB(B), normalA(nA), normalB(nB)
{}
Intersection::Contact& Intersection::Contact::swap()
{
	std::swap(contactPointA, contactPointB);
	std::swap(normalA, normalB);
	return *this;
}