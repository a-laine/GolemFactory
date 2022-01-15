#include <Physics/Collision.h>
#include "CollisionUtils.h"



//	Specialized functions : segment
bool Collision::collide_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b, CollisionReport* report)
{
	if (segment1a == segment1b)
		return collide_PointvsSegment(segment1a, segment2a, segment2b, report);
	else if (segment2a == segment2b)
		return collide_PointvsSegment(segment2a, segment1a, segment1b, report);

	glm::vec3 s1 = segment1b - segment1a;
	glm::vec3 s2 = segment2b - segment2a;
	glm::vec3 n = glm::cross(s1, s2);

	if (n == glm::vec3(0.f))	// parallel
		return false;
	else
	{
		std::pair<glm::vec3, glm::vec3> p = CollisionUtils::getSegmentsClosestSegment(segment1a, segment1b, segment2a, segment2b);
		if (glm::length2(p.first - p.second) > COLLISION_EPSILON * COLLISION_EPSILON)
			return false;
		else
		{
			if (report)
			{
				report->collision = true;
				report->points.push_back(p.first);
				report->depths.push_back(0.f);
				report->normal = n;
			}
			return true;
		}
	}
}

bool Collision::collide_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report)
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

		if (d1 >= d2 && d1 >= d3)
			return collide_SegmentvsSegment(segment1, segment2, triangle1, triangle2, report);
		else if (d2 >= d1 && d2 >= d3)
			return collide_SegmentvsSegment(segment1, segment2, triangle1, triangle3, report);
		else
			return collide_SegmentvsSegment(segment1, segment2, triangle3, triangle2, report);
	}

	//	compute intersection point between ray and plane
	if (segment2 == segment1)
		return collide_PointvsTriangle(segment1, triangle1, triangle2, triangle3, report);

	glm::vec3 s = segment2 - segment1;
	float dot = glm::dot(n, s);
	if (std::abs(dot) < COLLISION_EPSILON)
		return false; // segment parallel to triangle plane

	glm::vec3 u = glm::normalize(s);
	n = glm::normalize(dot > 0.f ? n : -n);

	float depth = glm::dot(n, triangle1 - segment1) / glm::dot(n, u);
	if (depth * depth > glm::length2(s) || depth < 0.f)
		return false; // too far or beind

	glm::vec3 intersection = segment1 + depth*u - triangle1;

	//	checking barycentric coordinates
	glm::vec2 bary = CollisionUtils::getBarycentricCoordinates(v1, v2, intersection);

	if (bary.x < 0.f || bary.y < 0.f || bary.x + bary.y > 1.f)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(intersection);

			float d1 = glm::dot(intersection - segment1, n);
			float d2 = glm::dot(intersection - segment2, n);
			if (std::abs(d1) < std::abs(d2))
			{
				report->depths.push_back(std::abs(d1));
				report->normal = d1 > 0.f ? -n : n;
			}
			else
			{
				report->depths.push_back(std::abs(d2));
				report->normal = d2 > 0.f ? -n : n;
			}
		}
		return true;
	}
}
bool Collision::collide_TrianglevsSegment(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report)
{
	return collide_SegmentvsTriangle(segment1, segment2, triangle1, triangle2, triangle3, report);
}

bool Collision::collide_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (segment2 == segment1) 
		return collide_PointvsSphere(segment1, sphereCenter, sphereRadius, report);

	glm::vec3 p = CollisionUtils::getSegmentClosestPoint(segment1, segment2, sphereCenter);
	glm::vec3 v = p - sphereCenter;
	float vv = glm::length2(v);

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = glm::sqrt(vv);
			report->collision = true;
			report->points.push_back(p);
			report->depths.push_back(sphereRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				glm::vec3 s = segment1 - segment2;
				report->normal = abs(s.x) > abs(s.z) ? glm::vec3(-s.y, s.x, 0.0) : glm::vec3(0.0, -s.z, s.y);
				report->normal = glm::normalize(report->normal);
			}
		}
		return true;
	}
}
bool Collision::collide_SpherevsSegment(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (segment2 == segment1)
		return collide_SpherevsPoint(segment1, sphereCenter, sphereRadius, report);

	glm::vec3 p = CollisionUtils::getSegmentClosestPoint(segment1, segment2, sphereCenter);
	glm::vec3 v = sphereCenter - p;
	float vv = glm::length2(v);

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(sphereRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				glm::vec3 s = segment1 - segment2;
				report->normal = abs(s.x) > abs(s.z) ? glm::vec3(-s.y, s.x, 0.0) : glm::vec3(0.0, -s.z, s.y);
				report->normal = glm::normalize(report->normal);
			}

			report->points.push_back(sphereCenter + sphereRadius * report->normal);
		}
		return true;
	}
}

bool Collision::collide_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (segment1 == segment2)
		return collide_PointvsCapsule(segment1, capsule1, capsule2, capsuleRadius, report);
	else if (capsule1 == capsule2)
		return collide_SegmentvsSphere(segment1, segment2, capsule1, capsuleRadius, report);

	std::pair<glm::vec3, glm::vec3> p = CollisionUtils::getSegmentsClosestSegment(segment1, segment2, capsule1, capsule2);
	glm::vec3 v = p.first - p.second;
	float vv = glm::length2(v);

	if (vv > capsuleRadius * capsuleRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = glm::sqrt(vv);
			report->collision = true;
			report->points.push_back(p.first);
			report->depths.push_back(capsuleRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				glm::vec3 s1 = segment1 - segment2;
				glm::vec3 s2 = capsule1 - capsule2;
				glm::vec3 n = glm::cross(s1, s2);

				if (glm::length2(n) < COLLISION_EPSILON)
					report->normal = abs(s1.x) > abs(s1.z) ? glm::vec3(-s1.y, s1.x, 0.0) : glm::vec3(0.0, -s1.z, s1.y);
				else
					report->normal = n;

				report->normal = glm::normalize(report->normal);
			}
		}
		return true;
	}
}
bool Collision::collide_CapsulevsSegment(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (segment1 == segment2)
		return collide_CapsulevsPoint(segment1, capsule1, capsule2, capsuleRadius, report);
	else if (capsule1 == capsule2)
		return collide_SpherevsSegment(segment1, segment2, capsule1, capsuleRadius, report);

	std::pair<glm::vec3, glm::vec3> p = CollisionUtils::getSegmentsClosestSegment(segment1, segment2, capsule1, capsule2);
	glm::vec3 v = p.second - p.first;
	float vv = glm::length2(v);

	if (vv > capsuleRadius * capsuleRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(capsuleRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				glm::vec3 s1 = segment1 - segment2;
				glm::vec3 s2 = capsule1 - capsule2;
				glm::vec3 n = glm::cross(s1, s2);

				if (glm::length2(n) < COLLISION_EPSILON)
					report->normal = abs(s1.x) > abs(s1.z) ? glm::vec3(-s1.y, s1.x, 0.0) : glm::vec3(0.0, -s1.z, s1.y);
				else
					report->normal = n;

				report->normal = glm::normalize(report->normal);
			}

			report->points.push_back(p.second + capsuleRadius * report->normal);
		}
		return true;
	}
}
//


