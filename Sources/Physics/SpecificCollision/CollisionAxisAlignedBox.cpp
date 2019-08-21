#include "CollisionAxisAlignedBox.h"
#include "CollisionOrientedBox.h"
#include "CollisionPoint.h"

#include "Physics/SpecificIntersection/IntersectionSegment.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>


//	Specialized functions : axis aligned box
bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return box1Min.x <= box2Max.x && box1Min.y <= box2Max.y && box1Min.z <= box2Max.z && box2Min.x <= box1Max.x && box2Min.y <= box1Max.y && box2Min.z <= box1Max.z;
}
bool Collision::collide_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	//	special case of obb/sphere
	glm::vec3 bcenter = 0.5f * (boxMax + boxMin);
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 p = sphereCenter - bcenter;

	if (p.x > bsize.x) p.x = bsize.x;
	else if (p.x < -bsize.x) p.x = -bsize.x;
	if (p.y > bsize.y) p.y = bsize.y;
	else if (p.y < -bsize.y) p.y = -bsize.y;
	if (p.z > bsize.z) p.z = bsize.z;
	else if (p.z < -bsize.z) p.z = -bsize.z;

	return collide_PointvsSphere(bcenter + p, sphereCenter, sphereRadius);
}
bool Collision::collide_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (capsule1 == capsule2) return collide_AxisAlignedBoxvsSphere(boxMin, boxMax, capsule1, capsuleRadius);
	else
	{
		Intersection::Contact contact = Intersection::intersect_SegmentvsAxisAlignedBox(capsule1, capsule2, boxMin, boxMax);
		if (glm::dot(contact.contactPointA - contact.contactPointB, contact.normalB) < 0)
			return true;
		else if (glm::length2(contact.contactPointA - contact.contactPointB) <= capsuleRadius * capsuleRadius)
			return true;
		else return false;
	}
}
//