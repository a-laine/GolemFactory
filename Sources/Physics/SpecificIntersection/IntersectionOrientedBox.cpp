#include "IntersectionOrientedBox.h"


#include "Physics/SpecificIntersection/IntersectionSegment.h"
#include "Physics/SpecificIntersection/IntersectionAxisAlignedBox.h"


Intersection::Contact Intersection::intersect_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	Contact contact;
	glm::vec3 p = glm::vec3(boxTranform * glm::vec4(sphereCenter, 1.f));
	contact = Intersection::intersect_AxisAlignedBoxvsSphere(boxMin, boxMax, p, sphereRadius);
	contact.contactPointB = glm::vec3(glm::inverse(boxTranform) * glm::vec4(contact.contactPointB, 1.f));
	return contact;
}
Intersection::Contact Intersection::intersect_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact;
	contact = Intersection::intersect_SegmentvsOrientedBox(capsule1, capsule2, boxTranform, boxMin, boxMax);
	contact.contactPointA = contact.contactPointA + capsuleRadius * contact.normalA;
	return contact.swap();
}


