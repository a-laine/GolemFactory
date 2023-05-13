#include <Physics/Collision.h>
#include "CollisionUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>


//	Specialized functions : point
bool Collision::collide_PointvsPoint(const vec4f& point1, const vec4f& point2, CollisionReport* report)
{
	bool collide = point1 == point2;
	if (collide && report)
	{
		report->collision = true;
		report->normal = vec4f(0, 1, 0, 0);
		report->points.push_back(point1);
		report->depths.push_back(0.f);
	}
	return collide;
}


bool Collision::collide_PointvsSegment(const vec4f& point, const vec4f& segment1, const vec4f& segment2, CollisionReport* report)
{
	if (segment1 == segment2) 
		return collide_PointvsPoint(point, segment1, report);
	else
	{
		vec4f v = point - segment1;
		vec4f s = segment2 - segment1;
		float dot = vec4f::dot(v, s);

		if (dot < -COLLISION_EPSILON)
			return false;
		else if (dot > s.getNorm() + COLLISION_EPSILON)
			return false;
		else
		{
			vec4f n = abs(s.x) > abs(s.z) ? vec4f(-s.y, s.x, 0, 0) : vec4f(0, -s.z, s.y, 0);
			dot = vec4f::dot(v, n);
			if (glm::abs(dot) > COLLISION_EPSILON)
				return false;
			else
			{
				if (report)
				{
					report->collision = true;
					report->points.push_back(point);
					report->depths.push_back(0.f);
					report->normal = n;
				}
				return true;
			}
		}
	}
}
bool Collision::collide_SegmentvsPoint(const vec4f& point, const vec4f& segment1, const vec4f& segment2, CollisionReport* report)
{
	return collide_PointvsSegment(point, segment1, segment2, report);
}


bool Collision::collide_PointvsTriangle(const vec4f& point, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report)
{
	//	check if point is coplanar to triangle
	vec4f u1 = triangle2 - triangle1;
	vec4f u2 = triangle3 - triangle1;
	vec4f n = vec4f::cross(u1, u2);
	vec4f p = point - triangle1;

	if (n.getNorm2() <= COLLISION_EPSILON) // flat triangle
	{
		vec4f u3 = triangle3 - triangle2;
		float d1 = u1.getNorm2();
		float d2 = u2.getNorm2();
		float d3 = u3.getNorm2();

		if (d1 >= d2 && d1 >= d3)
			return collide_PointvsSegment(point, triangle1, triangle2, report);
		else if (d2 >= d1 && d2 >= d3)
			return collide_PointvsSegment(point, triangle1, triangle3, report);
		else
			return collide_PointvsSegment(point, triangle3, triangle2, report);
	}
	else if (std::abs(vec4f::dot(p, n.getNormal())) <= COLLISION_EPSILON) // close enough to triangle plane
	{
		vec4f u = p - n * vec4f::dot(p, n);
		vec2f barry = CollisionUtils::getBarycentricCoordinates(u1, u2, u);
		float sum = barry.x + barry.y;

		if (barry.x < -COLLISION_EPSILON || barry.y < -COLLISION_EPSILON || sum > 1.f + COLLISION_EPSILON)
			return false;

		if (report)
		{
			report->collision = true;
			report->normal = vec4f::dot(p, n) >= 0.f ? n : -n;
			report->normal.normalize();
			report->points.push_back(point);
			report->depths.push_back(0.f);
		}
		return true;
	}
	return false;
}
bool Collision::collide_TrianglevsPoint(const vec4f& point, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report)
{
	return collide_PointvsTriangle(point, triangle1, triangle2, triangle3, report);
}


bool Collision::collide_PointvsOrientedBox(const vec4f& point, const mat4f& boxTranform, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report)
{
	vec4f bx = boxTranform[0];
	vec4f by = boxTranform[1];
	vec4f bz = boxTranform[2];

	vec4f center = boxTranform * (0.5f * (boxMax + boxMin));
	vec4f localSize = 0.5f * vec4f::abs(boxMax - boxMin);
	vec4f p = point - center;

	float px = vec4f::dot(p, bx);
	float py = vec4f::dot(p, by);
	float pz = vec4f::dot(p, bz);
	vec4f delta = localSize - vec4f::abs(vec4f(px, py, pz, 0));

	if (delta.x < 0.f || delta.y < 0.f || delta.z < 0.f)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(point);

			if (delta.x < delta.y && delta.x < delta.z)
			{
				report->depths.push_back(delta.x);
				report->normal = px > 0.f ? -bx : bx;
			}
			else if (delta.y < delta.x && delta.y < delta.z)
			{
				report->depths.push_back(delta.y);
				report->normal = py > 0.f ? -by : by;
			}
			else
			{
				report->depths.push_back(delta.z);
				report->normal = pz > 0.f ? -bz : bz;
			}
		}
		return true;
	}
}
bool Collision::collide_OrientedBoxvsPoint(const vec4f& point, const mat4f& boxTranform, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report)
{
	vec4f bx = boxTranform[0];
	vec4f by = boxTranform[1];
	vec4f bz = boxTranform[2];

	vec4f center = boxTranform * (0.5f * (boxMax + boxMin));
	vec4f localSize = 0.5f * vec4f::abs(boxMax - boxMin);
	vec4f p = point - center;

	float px = vec4f::dot(p, bx);
	float py = vec4f::dot(p, by);
	float pz = vec4f::dot(p, bz);
	vec4f delta = localSize - vec4f::abs(vec4f(px, py, pz, 0));

	if (delta.x < 0.f || delta.y < 0.f || delta.z < 0.f)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			if (delta.x < delta.y && delta.x < delta.z)
			{
				report->depths.push_back(delta.x);
				report->normal = px > 0.f ? bx : -bx;
				report->points.push_back(center + localSize.x * bx + py * by + pz * bz);
			}
			else if (delta.y < delta.x&& delta.y < delta.z)
			{
				report->depths.push_back(delta.y);
				report->normal = py > 0.f ? by : -by;
				report->points.push_back(center + px * bx + localSize.y * by + pz * bz);
			}
			else
			{
				report->depths.push_back(delta.z);
				report->normal = pz > 0.f ? bz : -bz;
				report->points.push_back(center + px * bx + py * by + localSize.z * bz);
			}
		}
		return true;
	}
}


bool Collision::collide_PointvsAxisAlignedBox(const vec4f& point, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report)
{
	if (vec4b::any(vec4f::lessThan(point, boxMin)) || vec4b::any(vec4f::greaterThan(point, boxMax)))
		return false;
	else 
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(point);

			vec4f v = point - 0.5f * (boxMax + boxMin);
			vec4f delta = 0.5f * vec4f::abs(boxMax - boxMin) - vec4f::abs(v);

			if (delta.x < delta.y && delta.x < delta.z)
			{
				report->depths.push_back(delta.x);
				report->normal = v.x > 0.f ? vec4f(-1, 0, 0, 0) : vec4f(1, 0, 0, 0);
			}
			else if (delta.y < delta.x && delta.y < delta.z)
			{
				report->depths.push_back(delta.y);
				report->normal = v.y > 0.f ? vec4f(0, -1, 0, 0) : vec4f(0, 1, 0, 0);
			}
			else
			{
				report->depths.push_back(delta.z);
				report->normal = v.z > 0.f ? vec4f(0, 0, -1, 0) : vec4f(0, 0, 1, 0);
			}
		}

		return true;
	}
}
bool Collision::collide_AxisAlignedBoxvsPoint(const vec4f& point, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report)
{
	if (vec4b::any(vec4f::lessThan(point, boxMin)) || vec4b::any(vec4f::greaterThan(point, boxMax)))
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			vec4f center = 0.5f * (boxMax + boxMin);
			vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
			vec4f v = point - center;
			vec4f delta = size - vec4f::abs(v);

			if (delta.x < delta.y && delta.x < delta.z)
			{
				report->depths.push_back(delta.x);
				report->normal = v.x > 0.f ? vec4f(1, 0, 0, 0) : vec4f(-1, 0, 0, 0);
				report->points.push_back(vec4f(center.x + size.x, point.y, point.z, 1));
			}
			else if (delta.y < delta.x && delta.y < delta.z)
			{
				report->depths.push_back(delta.y);
				report->normal = v.y > 0.f ? vec4f(0, 1, 0, 0) : vec4f(0, -1, 0, 0);
				report->points.push_back(vec4f(point.x, center.y + size.y, point.z, 1));
			}
			else
			{
				report->depths.push_back(delta.z);
				report->normal = v.z > 0.f ? vec4f(0, 0, 1, 0) : vec4f(0, 0, -1, 0);
				report->points.push_back(vec4f(point.x, point.y, center.z + size.z, 1));
			}
		}

		return true;
	}
}


bool Collision::collide_PointvsSphere(const vec4f& point, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	vec4f v = sphereCenter - point;
	float vv = v.getNorm2();
	if (vv < sphereRadius * sphereRadius)
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(point);
			report->depths.push_back(sphereRadius - std::sqrt(vv));

			if (vv > COLLISION_EPSILON * COLLISION_EPSILON)
				report->normal = v / vv;
			else
				report->normal = vec4f(0, 1, 0, 0);
		}
		return true;
	}
	return false;
}
bool Collision::collide_SpherevsPoint(const vec4f& point, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	vec4f v = sphereCenter - point;
	float vv = (sphereCenter - point).getNorm2();
	if (vv < sphereRadius * sphereRadius)
	{
		if (report)
		{
			report->collision = true;
			report->depths.push_back(sphereRadius - std::sqrt(vv));

			if (vv > COLLISION_EPSILON * COLLISION_EPSILON)
				report->normal = v / vv;
			else
				report->normal = vec4f(0, 1, 0, 0);

			report->points.push_back(sphereCenter + sphereRadius * report->normal);
		}
		return true;
	}
	return false;
}


bool Collision::collide_PointvsCapsule(const vec4f& point, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (capsule2 == capsule1) 
		return collide_PointvsSphere(point, capsule1, capsuleRadius, report);
	else if (capsuleRadius < COLLISION_EPSILON) 
		return collide_PointvsSegment(point, capsule1, capsule2, report);

	vec4f closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, point);
	vec4f v = point - closest;
	float vv = vec4f::dot(v, v);
	if (vv < capsuleRadius * capsuleRadius)
	{
		if (report)
		{
			report->collision = true;
			report->depths.push_back(capsuleRadius - std::sqrt(vv));
			report->points.push_back(point);

			if (vv < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				vec4f s = capsule2 - capsule1;
				report->normal = abs(s.x) > abs(s.z) ? vec4f(-s.y, s.x, 0, 0) : vec4f(0, -s.z, s.y, 0);
			}
			else
				report->normal = -v / vv;
		}
		return true;
	}
	return false;
}
bool Collision::collide_CapsulevsPoint(const vec4f& point, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (capsule2 == capsule1)
		return collide_SpherevsPoint(point, capsule1, capsuleRadius, report);
	else if (capsuleRadius < COLLISION_EPSILON)
		return collide_SegmentvsPoint(point, capsule1, capsule2, report);

	vec4f closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, point);
	vec4f v = point - closest;
	float vv = vec4f::dot(v, v);
	if (vv < capsuleRadius * capsuleRadius)
	{
		if (report)
		{
			report->collision = true;
			report->depths.push_back(capsuleRadius - std::sqrt(vv));

			if (vv < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				vec4f s = capsule2 - capsule1;
				report->normal = abs(s.x) > abs(s.z) ? vec4f(-s.y, s.x, 0, 0) : vec4f(0, -s.z, s.y, 0);
				report->normal.normalize();
			}
			else
				report->normal = v / vv;

			report->points.push_back(closest + capsuleRadius * report->normal);
		}
		return true;
	}
	return false;
}


bool Collision::collide_PointvsHull(const vec4f& point, const std::vector<vec4f>& hullPoints, const std::vector<vec4f>& hullNormals, const std::vector<unsigned short>& hullFaces, const mat4f& hullBase, CollisionReport* report)
{
	float dmax = std::numeric_limits<float>::min();
	unsigned int closestFace = 0;
	vec4f p = mat4f::inverse(hullBase) * point;
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		float d = vec4f::dot(hullNormals[i], (vec4f)p - hullPoints[hullFaces[3 * i]]);

		if ( d >= 0)
			return false;

		if (d > dmax)
		{
			closestFace = i;
			dmax = d;
		}
	}

	if (report)
	{
		report->collision = true;
		report->points.push_back(point);
		report->depths.push_back(-dmax);
		report->normal = hullBase * hullNormals[closestFace];
	}
	return true;
}
bool Collision::collide_HullvsPoint(const vec4f& point, const std::vector<vec4f>& hullPoints, const std::vector<vec4f>& hullNormals, const std::vector<unsigned short>& hullFaces, const mat4f& hullBase, CollisionReport* report)
{
	float dmax = std::numeric_limits<float>::min();
	unsigned int closestFace = 0;
	vec4f p = mat4f::inverse(hullBase) * point;
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		float d = vec4f::dot(hullNormals[i], (vec4f)p - hullPoints[hullFaces[3 * i]]);

		if (d >= 0)
			return false;

		if (d > dmax)
		{
			closestFace = i;
			dmax = d;
		}
	}

	if (report)
	{
		report->collision = true;
		report->depths.push_back(-dmax);
		report->normal = hullBase * hullNormals[closestFace];

		const vec4f t1 = hullBase * hullPoints[hullFaces[3 * closestFace]];
		const vec4f t2 = hullBase * hullPoints[hullFaces[3 * closestFace + 1]];
		const vec4f t3 = hullBase * hullPoints[hullFaces[3 * closestFace + 2]];
		vec4f u1 = t2 - t1;
		vec4f u2 = t3 - t1;

		vec4f u = p - report->normal * vec4f::dot(p, report->normal);
		vec2f barry = CollisionUtils::getBarycentricCoordinates(u1, u2, (vec4f)u);

		report->points.push_back(t1 + barry.x * u1 + barry.y * u2);
	}
	return true;
}
//
