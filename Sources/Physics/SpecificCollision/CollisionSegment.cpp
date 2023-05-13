#include <Physics/Collision.h>
#include "CollisionUtils.h"

#include "Math/TMath.h"

//	Specialized functions : segment
bool Collision::collide_SegmentvsSegment(const vec4f& segment1a, const vec4f& segment1b, const vec4f& segment2a, const vec4f& segment2b, CollisionReport* report)
{
	if (segment1a == segment1b)
		return collide_PointvsSegment(segment1a, segment2a, segment2b, report);
	else if (segment2a == segment2b)
		return collide_PointvsSegment(segment2a, segment1a, segment1b, report);

	vec4f s1 = segment1b - segment1a;
	vec4f s2 = segment2b - segment2a;
	vec4f n = vec4f::cross(s1, s2);

	if (n == vec4f::zero)	// parallel
		return false;
	else
	{
		std::pair<vec4f, vec4f> p = CollisionUtils::getSegmentsClosestSegment(segment1a, segment1b, segment2a, segment2b);
		if ((p.first - p.second).getNorm2() > COLLISION_EPSILON * COLLISION_EPSILON)
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

bool Collision::collide_SegmentvsTriangle(const vec4f& segment1, const vec4f& segment2, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report)
{
	//	begin and eliminate special cases
	vec4f v1 = triangle2 - triangle1;
	vec4f v2 = triangle3 - triangle1;
	vec4f n = vec4f::cross(v1, v2);

	if (n == vec4f::zero) // flat triangle
	{
		vec4f v3 = triangle3 - triangle2;
		float d1 = vec4f::dot(v1, v1);
		float d2 = vec4f::dot(v2, v2);
		float d3 = vec4f::dot(v3, v3);

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

	vec4f s = segment2 - segment1;
	float dot = vec4f::dot(n, s);
	if (std::abs(dot) < COLLISION_EPSILON)
		return false; // segment parallel to triangle plane

	vec4f u = s.getNormal();
	n = dot > 0.f ? n : -n;
	n.normalize();

	float depth = vec4f::dot(n, triangle1 - segment1) / vec4f::dot(n, u);
	if (depth * depth > s.getNorm2() || depth < 0.f)
		return false; // too far or beind

	vec4f intersection = segment1 + depth*u - triangle1;

	//	checking barycentric coordinates
	vec2f bary = CollisionUtils::getBarycentricCoordinates(v1, v2, intersection);

	if (bary.x < 0.f || bary.y < 0.f || bary.x + bary.y > 1.f)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(intersection);

			float d1 = vec4f::dot(intersection - segment1, n);
			float d2 = vec4f::dot(intersection - segment2, n);
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
bool Collision::collide_TrianglevsSegment(const vec4f& segment1, const vec4f& segment2, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report)
{
	return collide_SegmentvsTriangle(segment1, segment2, triangle1, triangle2, triangle3, report);
}

bool Collision::collide_SegmentvsSphere(const vec4f& segment1, const vec4f& segment2, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (segment2 == segment1) 
		return collide_PointvsSphere(segment1, sphereCenter, sphereRadius, report);

	vec4f p = CollisionUtils::getSegmentClosestPoint(segment1, segment2, sphereCenter);
	vec4f v = p - sphereCenter;
	float vv = v.getNorm2();

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = std::sqrt(vv);
			report->collision = true;
			report->points.push_back(p);
			report->depths.push_back(sphereRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				vec4f s = segment1 - segment2;
				report->normal = abs(s.x) > abs(s.z) ? vec4f(-s.y, s.x, 0.f, 0.f) : vec4f(0.0, -s.z, s.y, 0.f);
				report->normal.normalize();
			}
		}
		return true;
	}
}
bool Collision::collide_SpherevsSegment(const vec4f& segment1, const vec4f& segment2, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (segment2 == segment1)
		return collide_SpherevsPoint(segment1, sphereCenter, sphereRadius, report);

	vec4f p = CollisionUtils::getSegmentClosestPoint(segment1, segment2, sphereCenter);
	vec4f v = sphereCenter - p;
	float vv = v.getNorm2();

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(sphereRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				vec4f s = segment1 - segment2;
				report->normal = std::abs(s.x) > std::abs(s.z) ? vec4f(-s.y, s.x, 0.f, 0.f) : vec4f(0.f, -s.z, s.y, 0.f);
				report->normal.normalize();
			}

			report->points.push_back(sphereCenter + sphereRadius * report->normal);
		}
		return true;
	}
}

bool Collision::collide_SegmentvsCapsule(const vec4f& segment1, const vec4f& segment2, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (segment1 == segment2)
		return collide_PointvsCapsule(segment1, capsule1, capsule2, capsuleRadius, report);
	else if (capsule1 == capsule2)
		return collide_SegmentvsSphere(segment1, segment2, capsule1, capsuleRadius, report);

	std::pair<vec4f, vec4f> p = CollisionUtils::getSegmentsClosestSegment(segment1, segment2, capsule1, capsule2);
	vec4f v = p.first - p.second;
	float vv = v.getNorm2();

	if (vv > capsuleRadius * capsuleRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = std::sqrt(vv);
			report->collision = true;
			report->points.push_back(p.first);
			report->depths.push_back(capsuleRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				vec4f s1 = segment1 - segment2;
				vec4f s2 = capsule1 - capsule2;
				vec4f n = vec4f::cross(s1, s2);

				if (n.getNorm2() < COLLISION_EPSILON)
					report->normal = std::abs(s1.x) > std::abs(s1.z) ? vec4f(-s1.y, s1.x, 0.f, 0.f) : vec4f(0.f, -s1.z, s1.y, 0.f);
				else
					report->normal = n;

				report->normal.normalize();
			}
		}
		return true;
	}
}
bool Collision::collide_CapsulevsSegment(const vec4f& segment1, const vec4f& segment2, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (segment1 == segment2)
		return collide_CapsulevsPoint(segment1, capsule1, capsule2, capsuleRadius, report);
	else if (capsule1 == capsule2)
		return collide_SpherevsSegment(segment1, segment2, capsule1, capsuleRadius, report);

	std::pair<vec4f, vec4f> p = CollisionUtils::getSegmentsClosestSegment(segment1, segment2, capsule1, capsule2);
	vec4f v = p.second - p.first;
	float vv = v.getNorm2();

	if (vv > capsuleRadius * capsuleRadius)
		return false;
	else
	{
		if (report)
		{
			float distance = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(capsuleRadius - distance);

			if (vv > COLLISION_EPSILON)
				report->normal = v / distance;
			else
			{
				vec4f s1 = segment1 - segment2;
				vec4f s2 = capsule1 - capsule2;
				vec4f n = vec4f::cross(s1, s2);

				if (n.getNorm2() < COLLISION_EPSILON)
					report->normal = std::abs(s1.x) > std::abs(s1.z) ? vec4f(-s1.y, s1.x, 0.f, 0.f) : vec4f(0.f, -s1.z, s1.y, 0.f);
				else
					report->normal = n;

				report->normal.normalize();
			}

			report->points.push_back(p.second + capsuleRadius * report->normal);
		}
		return true;
	}
}
//


