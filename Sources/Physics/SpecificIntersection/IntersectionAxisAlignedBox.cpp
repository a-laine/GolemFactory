#include "IntersectionAxisAlignedBox.h"

#include <Physics/SpecificCollision/CollisionAxisAlignedBox.h>
#include <Physics/SpecificCollision/CollisionPoint.h>

#include "IntersectionPoint.h"
#include "IntersectionSegment.h"


Intersection::Contact Intersection::intersect_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	/*if (Collision::collide_AxisAlignedBoxvsAxisAlignedBox(box1Min, box1Max, box2Min, box2Max))
	{
		glm::vec3 bcenter1 = 0.5f * (box1Max + box1Min);
		glm::vec3 bcenter2 = 0.5f * (box2Max + box2Min);
		glm::vec3 bsize1 = 0.5f * (box1Max - box1Min);
		glm::vec3 bsize2 = 0.5f * (box2Max - box2Min);
		glm::vec3 d = glm::vec3(std::abs(bcenter1.x - bcenter2.x), std::abs(bcenter1.y - bcenter2.y), std::abs(bcenter1.z - bcenter2.z));
		glm::vec3 p1 = glm::vec3(0.f);;

		if (d.x > d.y && d.x > d.z)
		{
			if(bcenter1.x - bcenter2.x > 0)
				p1 = glm::vec3(bsize1.x, bcenter1.y - bcenter2.y, bcenter1.z - bcenter2.z);
			else
				p1 = glm::vec3(-bsize1.x, bcenter1.y - bcenter2.y, bcenter1.z - bcenter2.z);
		}
		else if (d.y > d.x && d.y > d.z)
		{
			if (bcenter1.y - bcenter2.y > 0)
				p1 = glm::vec3(bcenter1.x - bcenter2.x, bsize1.y, bcenter1.z - bcenter2.z);
			else
				p1 = glm::vec3(bcenter1.x - bcenter2.x, -bsize1.y, bcenter1.z - bcenter2.z);
		}
		else
		{
			if (bcenter1.z - bcenter2.z > 0)
				p1 = glm::vec3(bcenter1.x - bcenter2.x, bcenter1.y - bcenter2.y, bsize1.z);
			else
				p1 = glm::vec3(bcenter1.x - bcenter2.x, bcenter1.y - bcenter2.y, -bsize1.z);
		}

		Contact contact;
		contact = Intersection::intersect_PointvsAxisAlignedBox(p1, box2Min, box2Max);
	}
	else
	{

	}*/
	  
	return Intersection::Contact();
}
Intersection::Contact Intersection::intersect_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	Contact contact;
	contact = Intersection::intersect_PointvsAxisAlignedBox(sphereCenter, boxMin, boxMax);
	contact.contactPointA = contact.contactPointA + contact.normalA * sphereRadius;
	return contact.swap();
}
Intersection::Contact Intersection::intersect_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact;
	contact = Intersection::intersect_SegmentvsAxisAlignedBox(capsule1, capsule2, boxMin, boxMax);
	contact.contactPointA = contact.contactPointA + capsuleRadius * contact.normalA;
	return contact.swap();
}

