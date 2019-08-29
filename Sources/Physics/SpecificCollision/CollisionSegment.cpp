#include "CollisionSegment.h"
#include "CollisionPoint.h"
#include "CollisionUtils.h"

//	Specialized functions : segment
bool Collision::collide_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	if(segment1a == segment1b) return collide_PointvsSegment(segment1a, segment2a, segment2b);
	else if (segment2a == segment2b) return collide_PointvsSegment(segment2a, segment1a, segment1b);

	glm::vec3 s1 = segment1b - segment1a;
	glm::vec3 s2 = segment2b - segment2a;
	glm::vec3 n = glm::cross(s1, s2);

	if (n == glm::vec3(0.f))	// parallel
	{
		glm::vec3 u1 = glm::normalize(s1);
		glm::vec3 u3 = segment1a - segment2a;
		return glm::length2(u3 - u1 * std::abs(glm::dot(u3, u1))) <= COLLISION_EPSILON*COLLISION_EPSILON;
	}
	else
	{
		std::pair<glm::vec3, glm::vec3> p = getSegmentsClosestSegment(segment1a, segment1b, segment2a, segment2b);
		return glm::length2(p.first - p.second) <= COLLISION_EPSILON*COLLISION_EPSILON;
	}
}
bool Collision::collide_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, glm::vec3& intersection)
{
	//	begin and eliminate special cases
	glm::vec3 v1 = triangle2 - triangle1;
	glm::vec3 v2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(v1, v2);

	if (n == glm::vec3(0.f)) // flat triangle
	{
		glm::vec3 v3 = triangle3 - triangle2;
		float d1 = glm::dot(v1, v1);
		float d2 = glm::dot(v2, v2);
		float d3 = glm::dot(v3, v3);

		if (d1 >= d2 && d1 >= d3) return collide_SegmentvsSegment(segment1, segment2, triangle1, triangle2);
		else if (d2 >= d1 && d2 >= d3) return collide_SegmentvsSegment(segment1, segment2, triangle1, triangle3);
		else return collide_SegmentvsSegment(segment1, segment2, triangle3, triangle2);
	}

	//	compute intersection point between ray and plane
	glm::vec3 s = segment2 - segment1;
	if (s == glm::vec3(0.f)) return collide_PointvsTriangle(segment1, triangle1, triangle2, triangle3);

	glm::normalize(n);
	if (glm::dot(n, s) == 0.f) return false; // segment parallel to triangle plane
	glm::vec3 u = glm::normalize(s);
	if (glm::dot(n, s) > 0.f) n *= -1.f;

	float depth = glm::dot(n, triangle1 - segment1) / glm::dot(n, u);
	if (depth > glm::length(s) || depth < 0.f) return false; // too far or beind
	intersection = segment1 + depth*u - triangle1;

	//	checking barycentric coordinates
	glm::vec2 bary = getBarycentricCoordinates(v1, v2, intersection);
	return !(bary.x < 0.f || bary.y < 0.f || bary.x + bary.y > 1.f);
}
bool Collision::collide_SegmentvsOrientedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	//http://www.opengl-tutorial.org/fr/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/

	if (segment2 == segment1) return collide_PointvsOrientedBox(segment1, boxTranform, boxMin, boxMax);
	glm::vec3 u = glm::normalize(segment2 - segment1);
	glm::vec3 delta = glm::vec3(boxTranform[3]) - segment1;

	float tmin = 0.f;
	float tmax = std::numeric_limits<float>::max();

	//	test on the two axis of local x
	glm::vec3 bx = glm::vec3(boxTranform[0]);
	glm::normalize(bx);
	float e = glm::dot(bx, delta);
	if (glm::dot(bx, u) == 0.f) // segment parallel to selected plane
	{
		if (-e + boxMin.x > 0.0f || -e + boxMax.x < 0.0f) return false;
	}
	else
	{
		float t1 = (e + boxMin.x) / glm::dot(bx, u); // Intersection with the "left" plane
		float t2 = (e + boxMax.x) / glm::dot(bx, u); // Intersection with the "right" plane
		if (t1 > t2) std::swap(t1, t2);

		if (t2 < tmax) tmax = t2;
		if (t1 > tmin) tmin = t1;
		if (tmax < tmin) return false;
	}

	//	test on the two axis of local y
	glm::vec3 by = glm::vec3(boxTranform[1]);
	glm::normalize(by);
	e = glm::dot(by, delta);
	if (glm::dot(by, u) == 0.f) // segment parallel to selected plane
	{
		if (-e + boxMin.y > 0.0f || -e + boxMax.y < 0.0f) return false;
	}
	else
	{
		float t1 = (e + boxMin.y) / glm::dot(by, u); // Intersection with the "left" plane
		float t2 = (e + boxMax.y) / glm::dot(by, u); // Intersection with the "right" plane
		if (t1 > t2) std::swap(t1, t2);

		if (t2 < tmax) tmax = t2;
		if (t1 > tmin) tmin = t1;
		if (tmax < tmin) return false;
	}

	//	test on the two axis of local z
	glm::vec3 bz = glm::vec3(boxTranform[2]);
	glm::normalize(bz);
	e = glm::dot(bz, delta);
	if (glm::dot(bz, u) == 0.f) // segment parallel to selected plane
	{
		if (-e + boxMin.z > 0.0f || -e + boxMax.z < 0.0f) return false;
	}
	else
	{
		float t1 = (e + boxMin.z) / glm::dot(bz, u); // Intersection with the "left" plane
		float t2 = (e + boxMax.z) / glm::dot(bz, u); // Intersection with the "right" plane
		if (t1 > t2) std::swap(t1, t2);

		if (t2 < tmax) tmax = t2;
		if (t1 > tmin) tmin = t1;
		if (tmax < tmin) return false;
	}
	return tmin <= glm::length(segment2 - segment1);
}
bool Collision::collide_SegmentvsAxisAlignedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	if (segment2 == segment1) return collide_PointvsAxisAlignedBox(segment1, boxMin, boxMax);

	glm::vec3 s = segment2 - segment1;
	glm::vec3 u = glm::normalize(s);
	glm::vec3 t1 = (boxMin - segment1) / u;
	glm::vec3 t2 = (boxMax - segment1) / u;
	float tnear = glm::compMax(glm::min(t1, t2));
	float tfar = glm::compMin(glm::max(t1, t2));
	if (tfar >= tnear && tfar >= 0 && tnear <= glm::length(s)) return true;
	else return false;
}
bool Collision::collide_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (segment2 == segment1) return collide_PointvsSphere(segment1, sphereCenter, sphereRadius);
	glm::vec3 p = getSegmentClosestPoint(segment1, segment2, sphereCenter);
	return glm::length2(p - sphereCenter) < sphereRadius * sphereRadius + COLLISION_EPSILON;
}
bool Collision::collide_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (segment1 == segment2) return collide_PointvsSegment(segment1, capsule1, capsule2);
	else if (capsule1 == capsule2) return collide_SegmentvsSphere(segment1, segment2, capsule1, capsuleRadius);

	std::pair<glm::vec3, glm::vec3> p = getSegmentsClosestSegment(segment1, segment2, capsule1, capsule2);
	return glm::length2(p.first - p.second) < capsuleRadius * capsuleRadius + COLLISION_EPSILON;
}
//


