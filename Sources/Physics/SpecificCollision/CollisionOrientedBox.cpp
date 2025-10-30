#include "CollisionUtils.h"
#include <Physics/Collision.h>


//	Specialized functions : oriented box
/*bool Collision::collide_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max)
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
	if      (std::abs(glm::dot(xb1, distance)) > projectHalfBox(xb1, sb1) + projectHalfBox(xb1, sb2)) return false;
	else if (std::abs(glm::dot(yb1, distance)) > projectHalfBox(yb1, sb1) + projectHalfBox(yb1, sb2)) return false;
	else if (std::abs(glm::dot(zb1, distance)) > projectHalfBox(zb1, sb1) + projectHalfBox(zb1, sb2)) return false;
	else if (std::abs(glm::dot(xb2, distance)) > projectHalfBox(xb2, sb1) + projectHalfBox(xb2, sb2)) return false;
	else if (std::abs(glm::dot(yb2, distance)) > projectHalfBox(yb2, sb1) + projectHalfBox(yb2, sb2)) return false;
	else if (std::abs(glm::dot(zb2, distance)) > projectHalfBox(zb2, sb1) + projectHalfBox(zb2, sb2)) return false;

	//	secondary axis checking
	glm::vec3 xb1xb2 = glm::cross(xb1, xb2); if (glm::dot(xb1xb2, xb1xb2) > COLLISION_EPSILON) xb1xb2 = glm::normalize(xb1xb2); else xb1xb2 = glm::vec3(1, 0, 0);
	glm::vec3 xb1yb2 = glm::cross(xb1, yb2); if (glm::dot(xb1yb2, xb1yb2) > COLLISION_EPSILON) xb1yb2 = glm::normalize(xb1yb2); else xb1yb2 = glm::vec3(1, 0, 0);
	glm::vec3 xb1zb2 = glm::cross(xb1, zb2); if (glm::dot(xb1zb2, xb1zb2) > COLLISION_EPSILON) xb1zb2 = glm::normalize(xb1zb2); else xb1xb2 = glm::vec3(1, 0, 0);
	glm::vec3 yb1xb2 = glm::cross(yb1, xb2); if (glm::dot(yb1xb2, yb1xb2) > COLLISION_EPSILON) yb1xb2 = glm::normalize(yb1xb2); else yb1xb2 = glm::vec3(1, 0, 0);
	glm::vec3 yb1yb2 = glm::cross(yb1, yb2); if (glm::dot(yb1yb2, yb1yb2) > COLLISION_EPSILON) yb1yb2 = glm::normalize(yb1yb2); else yb1yb2 = glm::vec3(1, 0, 0);
	glm::vec3 yb1zb2 = glm::cross(yb1, zb2); if (glm::dot(yb1zb2, yb1zb2) > COLLISION_EPSILON) yb1zb2 = glm::normalize(yb1zb2); else yb1zb2 = glm::vec3(1, 0, 0);
	glm::vec3 zb1xb2 = glm::cross(zb1, xb2); if (glm::dot(zb1xb2, zb1xb2) > COLLISION_EPSILON) zb1xb2 = glm::normalize(zb1xb2); else zb1xb2 = glm::vec3(1, 0, 0);
	glm::vec3 zb1yb2 = glm::cross(zb1, yb2); if (glm::dot(zb1yb2, zb1yb2) > COLLISION_EPSILON) zb1yb2 = glm::normalize(zb1yb2); else zb1yb2 = glm::vec3(1, 0, 0);
	glm::vec3 zb1zb2 = glm::cross(zb1, zb2); if (glm::dot(zb1zb2, zb1zb2) > COLLISION_EPSILON) zb1zb2 = glm::normalize(zb1zb2); else zb1zb2 = glm::vec3(1, 0, 0);

	//	second test pass
	if      (std::abs(glm::dot(xb1xb2, distance)) > projectHalfBox(xb1xb2, sb1) + projectHalfBox(xb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(xb1yb2, distance)) > projectHalfBox(xb1yb2, sb1) + projectHalfBox(xb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(xb1zb2, distance)) > projectHalfBox(xb1zb2, sb1) + projectHalfBox(xb1zb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1xb2, distance)) > projectHalfBox(yb1xb2, sb1) + projectHalfBox(yb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1yb2, distance)) > projectHalfBox(yb1yb2, sb1) + projectHalfBox(yb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1zb2, distance)) > projectHalfBox(yb1zb2, sb1) + projectHalfBox(yb1zb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1xb2, distance)) > projectHalfBox(zb1xb2, sb1) + projectHalfBox(zb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1yb2, distance)) > projectHalfBox(zb1yb2, sb1) + projectHalfBox(zb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1zb2, distance)) > projectHalfBox(zb1zb2, sb1) + projectHalfBox(zb1zb2, sb2)) return false;
	else return true;
}
bool Collision::collide_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return collide_OrientedBoxvsOrientedBox(box1Tranform, box1Min, box1Max, glm::mat4(1.f), box2Min, box2Max);
}*/
/*bool Collision2::collide_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
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

	return false;// collide_PointvsSphere(boxClosestPoint, sphereCenter, sphereRadius);
}
bool Collision2::collide_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (capsule1 == capsule2) return collide_OrientedBoxvsSphere(boxTranform, boxMin, boxMax, capsule1, capsuleRadius);

	glm::vec3 bcenter = 0.5f * (glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) + glm::vec3(boxTranform*glm::vec4(boxMin, 1.f)));
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 bx = glm::vec3(boxTranform[0]);	// box local x
	glm::vec3 by = glm::vec3(boxTranform[1]);	// box local y
	glm::vec3 bz = glm::vec3(boxTranform[2]);	// box local z

	glm::vec3 closestBoxPoint = bcenter;
	glm::vec3 closestSegmentPoint = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	float d = glm::dot(bx, closestSegmentPoint - closestBoxPoint);
	if (d > bsize.x) d = bsize.x;
	else if (d < -bsize.x) d = -bsize.x;
	closestBoxPoint += d* bx;
	closestSegmentPoint = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	d = glm::dot(by, closestSegmentPoint - closestBoxPoint);
	if (d > bsize.y) d = bsize.y;
	else if (d < -bsize.y) d = -bsize.y;
	closestBoxPoint += d* by;
	closestSegmentPoint = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	d = glm::dot(bz, closestSegmentPoint - closestBoxPoint);
	if (d > bsize.z) d = bsize.z;
	else if (d < -bsize.z) d = -bsize.z;
	closestBoxPoint += d* bz;
	closestSegmentPoint = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, closestBoxPoint);

	return glm::length(closestBoxPoint - closestSegmentPoint) <= capsuleRadius;
}*/


bool Collision::raycast_OrientedBox(const vec4f& worldRayOrigin, const vec4f& worldRayDirection, const mat4f& boxTransform, const vec4f& boxMin, const vec4f& boxMax, RaycastReport* report)
{
	mat4f invTransform = mat4f::inverse(boxTransform);
	vec4f rayEnd = worldRayOrigin + worldRayDirection.w * worldRayDirection;
	rayEnd.w = 1.f;
	vec4f s1 = invTransform * worldRayOrigin;
	vec4f s2 = invTransform * rayEnd;

	// test if already in box
	if (vec4b::all(vec4f::greaterThan(s1, boxMin)) && vec4b::all(vec4f::lessThan(s1, boxMax)))
	{
		if (report)
		{
			report->m_distance = 0.f;
			report->m_intersection = worldRayOrigin;
			report->m_intersection.w = 1;

			vec4f center = 0.5f * (boxMax + boxMin);
			vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
			vec4f v = s1 - center;
			vec4f delta = size - vec4f::abs(v);

			if (delta.x < delta.y&& delta.x < delta.z)
				report->m_normal = v.x > 0.f ? vec4f(1, 0, 0, 0) : vec4f(-1, 0, 0, 0);
			else if (delta.y < delta.x && delta.y < delta.z)
				report->m_normal = v.y > 0.f ? vec4f(0, 1, 0, 0) : vec4f(0, -1, 0, 0);
			else
				report->m_normal = v.z > 0.f ? vec4f(0, 0, 1, 0) : vec4f(0, 0, -1, 0);
			report->m_normal = boxTransform * report->m_normal;
		}
		return true;
	}

	vec4f s = s2 - s1;
	float rayLength = s.getNorm();
	if (rayLength < COLLISION_EPSILON)
		return false;

	// ray box regular
	vec4f rayDirection = s * (1.f / rayLength);
	vec4f rayInvDirection = vec4f::zero;
	rayInvDirection.x = std::abs(rayDirection.x) > COLLISION_EPSILON * COLLISION_EPSILON ? 1.f / rayDirection.x : 1E12f;
	rayInvDirection.y = std::abs(rayDirection.y) > COLLISION_EPSILON * COLLISION_EPSILON ? 1.f / rayDirection.y : 1E12f;
	rayInvDirection.z = std::abs(rayDirection.z) > COLLISION_EPSILON * COLLISION_EPSILON ? 1.f / rayDirection.z : 1E12f;

	vec4f t0 = (boxMax - s1) * rayInvDirection;
	vec4f t1 = (boxMin - s1) * rayInvDirection;
	vec4f tmin = vec4f::min(t0, t1);
	vec4f tmax = vec4f::max(t0, t1);
	float distanceB = std::min(std::min(tmax.x, tmax.y), tmax.z);
	float distanceA = std::max(std::max(tmin.x, tmin.y), tmin.z);
	if (distanceB < distanceA + COLLISION_EPSILON || distanceA > rayLength)
		return false;

	if (report)
	{
		vec4f localIntersection = s1 + distanceA * rayDirection;
		report->m_intersection = boxTransform * localIntersection;
		report->m_intersection.w = 1;
		report->m_distance = (report->m_intersection - worldRayOrigin).getNorm();

		vec4f center = 0.5f * (boxMax + boxMin);
		vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
		vec4f v = localIntersection - center;
		vec4f delta = vec4f::abs(v) / size;

		if (delta.x < delta.y && delta.x < delta.z)
			report->m_normal = v.x > 0.f ? vec4f(1, 0, 0, 0) : vec4f(-1, 0, 0, 0);
		else if (delta.y < delta.x && delta.y < delta.z)
			report->m_normal = v.y > 0.f ? vec4f(0, 1, 0, 0) : vec4f(0, -1, 0, 0);
		else
			report->m_normal = v.z > 0.f ? vec4f(0, 0, 1, 0) : vec4f(0, 0, -1, 0);
		report->m_normal = boxTransform * report->m_normal;
		report->m_normal.normalize();
	}
	return true;
}

bool Collision::raycast_Triangle(const vec4f& rayOrigin, const vec4f& rayDirection, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, bool discardBackface, RaycastReport* report)
{
	constexpr float eps = 1E-06f;
	vec4f direction = rayDirection;         direction.w = 0.f;
	vec4f edge1 = triangle2 - triangle1;    edge1.w = 0.f;
	vec4f edge2 = triangle3 - triangle1;    edge2.w = 0.f;
	vec4f h = vec4f::cross(direction, edge2);
	float a = vec4f::dot(h, edge1);
	if ((discardBackface ? a : std::abs(a)) < eps)
		return false;

	float f = 1.f / a;
	vec4f s = rayOrigin - triangle1;
	s.w = 0.f;
	float u = f * vec4f::dot(s, h);
	if (u < 0.f || u > 1.f)
		return false;

	vec4f q = vec4f::cross(s, edge1);
	float v = f * vec4f::dot(q, direction);
	if (v < 0.f || u + v > 1.f)
		return false;

	float t = f * vec4f::dot(edge2, q);
	if (t < 0.f || t >= rayDirection.w)
		return false;

	if (report)
	{
		report->m_normal = vec4f::cross(edge1, edge2);
		report->m_normal.normalize();
		report->m_distance = t;
		report->m_intersection = rayOrigin + t * direction;
	}
	return true;

	/*vec4f v1 = triangle2 - triangle1;
	vec4f v2 = triangle3 - triangle1;
	vec4f n = vec4f::cross(v1, v2);

	vec4f s = segment2 - segment1;
	float dot = vec4f::dot(n, s);
	if (std::abs(dot) < COLLISION_EPSILON * COLLISION_EPSILON)
		return false; // segment parallel to triangle plane

	vec4f u = s.getNormal();
	n = dot > 0.f ? n : -n;
	n.normalize();

	float depth = vec4f::dot(n, triangle1 - segment1) / vec4f::dot(n, u);
	if (depth * depth > s.getNorm2() || depth < 0.f)
		return false; // too far or beind

	vec4f intersection = segment1 + depth * u - triangle1;

	//	checking barycentric coordinates
	vec2f bary = CollisionUtils::getBarycentricCoordinates(v1, v2, intersection);
	if (bary.x < 0.f || bary.y < 0.f || bary.x + bary.y > 1.f)
		return false;
	else
	{
		if (report)
		{
			report->m_normal = n;
			report->m_intersection = segment1 + depth * u;
			report->m_distance = depth;
		}
		return true;
	}*/
}
//

