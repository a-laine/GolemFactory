#include "IntersectionOrientedBox.h"

#include "IntersectionPoint.h"
#include "IntersectionSegment.h"


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
	glm::vec3 p = glm::vec3(glm::inverse(boxTranform) * glm::vec4(sphereCenter, 1.f));
	Contact contact = Intersection::intersect_PointvsOrientedBox(sphereCenter, boxTranform, boxMin, boxMax);
	contact.contactPointA = contact.contactPointA + sphereRadius * contact.normalA;
	return contact.swap();
}
Intersection::Contact Intersection::intersect_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact;
	contact = Intersection::intersect_SegmentvsOrientedBox(capsule1, capsule2, boxTranform, boxMin, boxMax);
	contact.contactPointA = contact.contactPointA + capsuleRadius * contact.normalA;
	return contact.swap();
}


