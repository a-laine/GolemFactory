#include "CollisionOrientedBox.h"
#include "CollisionPoint.h"
#include "CollisionUtils.h"


//	Specialized functions : oriented box
bool Collision::collide_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	//	axis to check in absolute base
	glm::vec3 xb1 = glm::vec3(box1Tranform[0]);
	glm::vec3 yb1 = glm::vec3(box1Tranform[1]);
	glm::vec3 zb1 = glm::vec3(box1Tranform[2]);
	glm::vec3 xb2 = glm::vec3(box2Tranform[0]);
	glm::vec3 yb2 = glm::vec3(box2Tranform[1]);
	glm::vec3 zb2 = glm::vec3(box2Tranform[2]);

	//	distance between objects centroids and boxes diagonal sizes (half of them)
	glm::vec3 distance = glm::vec3(box1Tranform*glm::vec4(0.5f*(box1Min + box1Max), 1.f)) - glm::vec3(box2Tranform*glm::vec4(0.5f*(box2Min + box2Max), 1.f));
	glm::vec3 sb1 = 0.5f * glm::abs(box1Max - box1Min);
	glm::vec3 sb2 = 0.5f * glm::abs(box2Max - box2Min);

	//	first test pass
	if (std::abs(glm::dot(xb1, distance)) > projectHalfBox(xb1, sb1) + projectHalfBox(xb1, sb2)) return false;
	else if (std::abs(glm::dot(yb1, distance)) > projectHalfBox(yb1, sb1) + projectHalfBox(yb1, sb2)) return false;
	else if (std::abs(glm::dot(zb1, distance)) > projectHalfBox(zb1, sb1) + projectHalfBox(zb1, sb2)) return false;
	else if (std::abs(glm::dot(xb2, distance)) > projectHalfBox(xb2, sb1) + projectHalfBox(xb2, sb2)) return false;
	else if (std::abs(glm::dot(yb2, distance)) > projectHalfBox(yb2, sb1) + projectHalfBox(yb2, sb2)) return false;
	else if (std::abs(glm::dot(zb2, distance)) > projectHalfBox(zb2, sb1) + projectHalfBox(zb2, sb2)) return false;

	//	secondary axis checking
	/*glm::vec3 xb1xb2 = glm::normalize(glm::cross(xb1, xb2));
	glm::vec3 xb1yb2 = glm::normalize(glm::cross(xb1, yb2));
	glm::vec3 xb1zb2 = glm::normalize(glm::cross(xb1, zb2));
	glm::vec3 yb1xb2 = glm::normalize(glm::cross(yb1, xb2));
	glm::vec3 yb1yb2 = glm::normalize(glm::cross(yb1, yb2));
	glm::vec3 yb1zb2 = glm::normalize(glm::cross(yb1, zb2));
	glm::vec3 zb1xb2 = glm::normalize(glm::cross(zb1, xb2));
	glm::vec3 zb1yb2 = glm::normalize(glm::cross(zb1, yb2));
	glm::vec3 zb1zb2 = glm::normalize(glm::cross(zb1, zb2));

	//	second test pass
	if (std::abs(glm::dot(xb1xb2, distance)) > projectHalfBox(xb1xb2, sb1) + projectHalfBox(xb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(xb1yb2, distance)) > projectHalfBox(xb1yb2, sb1) + projectHalfBox(xb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(xb1zb2, distance)) > projectHalfBox(xb1zb2, sb1) + projectHalfBox(xb1zb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1xb2, distance)) > projectHalfBox(yb1xb2, sb1) + projectHalfBox(yb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1yb2, distance)) > projectHalfBox(yb1yb2, sb1) + projectHalfBox(yb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1zb2, distance)) > projectHalfBox(yb1zb2, sb1) + projectHalfBox(yb1zb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1xb2, distance)) > projectHalfBox(zb1xb2, sb1) + projectHalfBox(zb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1yb2, distance)) > projectHalfBox(zb1yb2, sb1) + projectHalfBox(zb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1zb2, distance)) > projectHalfBox(zb1zb2, sb1) + projectHalfBox(zb1zb2, sb2)) return false;*/
	else return true;
}
bool Collision::collide_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return collide_OrientedBoxvsOrientedBox(box1Tranform, box1Min, box1Max, glm::mat4(1.f), box2Min, box2Max);
}
bool Collision::collide_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	//	for help read https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
	//	line 165

	glm::vec3 bcenter = 0.5f * (glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) + glm::vec3(boxTranform*glm::vec4(boxMin, 1.f)));
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 bx = glm::vec3(boxTranform[0]);	// box local x
	glm::vec3 by = glm::vec3(boxTranform[1]);	// box local y
	glm::vec3 bz = glm::vec3(boxTranform[2]);	// box local z

	glm::vec3 p = sphereCenter - bcenter;
	glm::vec3 boxClosestPoint = bcenter;

	float d = glm::dot(bx, p);
	if (d > bsize.x) d = bsize.x;
	else if (d < -bsize.x) d = -bsize.x;
	boxClosestPoint += d* bx;

	d = glm::dot(by, p);
	if (d > bsize.y) d = bsize.y;
	else if (d < -bsize.y) d = -bsize.y;
	boxClosestPoint += d* by;

	d = glm::dot(bz, p);
	if (d > bsize.z) d = bsize.z;
	else if (d < -bsize.z) d = -bsize.z;
	boxClosestPoint += d* bz;

	return collide_PointvsSphere(boxClosestPoint, sphereCenter, sphereRadius);
}
bool Collision::collide_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (capsule1 == capsule2) return collide_OrientedBoxvsSphere(boxTranform, boxMin, boxMax, capsule1, capsuleRadius);

	glm::vec3 bcenter = 0.5f * (glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) + glm::vec3(boxTranform*glm::vec4(boxMin, 1.f)));
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 bx = glm::vec3(boxTranform[0]);	// box local x
	glm::vec3 by = glm::vec3(boxTranform[1]);	// box local y
	glm::vec3 bz = glm::vec3(boxTranform[2]);	// box local z

	glm::vec3 closestBoxPoint = bcenter;
	glm::vec3 closestSegmentPoint = getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	float d = glm::dot(bx, closestSegmentPoint - closestBoxPoint);
	if (d > bsize.x) d = bsize.x;
	else if (d < -bsize.x) d = -bsize.x;
	closestBoxPoint += d* bx;
	closestSegmentPoint = getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	d = glm::dot(by, closestSegmentPoint - closestBoxPoint);
	if (d > bsize.y) d = bsize.y;
	else if (d < -bsize.y) d = -bsize.y;
	closestBoxPoint += d* by;
	closestSegmentPoint = getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	d = glm::dot(bz, closestSegmentPoint - closestBoxPoint);
	if (d > bsize.z) d = bsize.z;
	else if (d < -bsize.z) d = -bsize.z;
	closestBoxPoint += d* bz;
	closestSegmentPoint = getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	return glm::length(closestBoxPoint - closestSegmentPoint) <= capsuleRadius;
}
//

