#include <Physics/Collision.h>
#include "CollisionUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>


//	Specialized functions : point
bool Collision::collide_PointvsPoint(const glm::vec4& point1, const glm::vec4& point2, CollisionReport* report)
{
	bool collide = point1 == point2;
	if (collide && report)
	{
		report->collision = true;
		report->normal = glm::vec4(0, 1, 0, 0);
		report->points.push_back(point1);
		report->depths.push_back(0.f);
	}
	return collide;
}


bool Collision::collide_PointvsSegment(const glm::vec4& point, const glm::vec4& segment1, const glm::vec4& segment2, CollisionReport* report)
{
	if (segment1 == segment2) 
		return collide_PointvsPoint(point, segment1, report);
	else
	{
		glm::vec4 v = point - segment1;
		glm::vec4 s = segment2 - segment1;
		float dot = glm::dot(v, s);

		if (dot < -COLLISION_EPSILON)
			return false;
		else if (dot > glm::length2(s) + COLLISION_EPSILON)
			return false;
		else
		{
			glm::vec4 n = abs(s.x) > abs(s.z) ? glm::vec4(-s.y, s.x, 0, 0) : glm::vec4(0, -s.z, s.y, 0);
			dot = glm::dot(v, n);
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
bool Collision::collide_SegmentvsPoint(const glm::vec4& point, const glm::vec4& segment1, const glm::vec4& segment2, CollisionReport* report)
{
	return collide_PointvsSegment(point, segment1, segment2, report);
}


bool Collision::collide_PointvsTriangle(const glm::vec4& point, const glm::vec4& triangle1, const glm::vec4& triangle2, const glm::vec4& triangle3, CollisionReport* report)
{
	//	check if point is coplanar to triangle
	glm::vec4 u1 = triangle2 - triangle1;
	glm::vec4 u2 = triangle3 - triangle1;
	glm::vec4 n = glm::cross(u1, u2);
	glm::vec4 p = point - triangle1;

	if (glm::length2(n) <= COLLISION_EPSILON) // flat triangle
	{
		glm::vec4 u3 = triangle3 - triangle2;
		float d1 = glm::length2(u1);
		float d2 = glm::length2(u2);
		float d3 = glm::length2(u3);

		if (d1 >= d2 && d1 >= d3)
			return collide_PointvsSegment(point, triangle1, triangle2, report);
		else if (d2 >= d1 && d2 >= d3)
			return collide_PointvsSegment(point, triangle1, triangle3, report);
		else
			return collide_PointvsSegment(point, triangle3, triangle2, report);
	}
	else if (std::abs(glm::dot(p, glm::normalize(n))) <= COLLISION_EPSILON) // close enough to triangle plane
	{
		glm::vec4 u = p - n * glm::dot(p, n);
		glm::vec2 barry = CollisionUtils::getBarycentricCoordinates(u1, u2, u);
		float sum = barry.x + barry.y;

		if (barry.x < -COLLISION_EPSILON || barry.y < -COLLISION_EPSILON || sum > 1.f + COLLISION_EPSILON)
			return false;

		if (report)
		{
			report->collision = true;
			report->normal = glm::dot(p, n) >= 0.f ? n : -n;
			report->normal = glm::normalize(report->normal);
			report->points.push_back(point);
			report->depths.push_back(0.f);
		}
		return true;
	}
	return false;
}
bool Collision::collide_TrianglevsPoint(const glm::vec4& point, const glm::vec4& triangle1, const glm::vec4& triangle2, const glm::vec4& triangle3, CollisionReport* report)
{
	return collide_PointvsTriangle(point, triangle1, triangle2, triangle3, report);
}


bool Collision::collide_PointvsOrientedBox(const glm::vec4& point, const glm::mat4& boxTranform, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report)
{
	glm::vec4 bx = boxTranform[0];
	glm::vec4 by = boxTranform[1];
	glm::vec4 bz = boxTranform[2];

	glm::vec4 center = boxTranform * (0.5f * (boxMax + boxMin));
	glm::vec4 localSize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec4 p = point - center;

	float px = glm::dot(p, bx);
	float py = glm::dot(p, by);
	float pz = glm::dot(p, bz);
	glm::vec4 delta = localSize - glm::abs(glm::vec4(px, py, pz, 0));

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
bool Collision::collide_OrientedBoxvsPoint(const glm::vec4& point, const glm::mat4& boxTranform, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report)
{
	glm::vec4 bx = boxTranform[0];
	glm::vec4 by = boxTranform[1];
	glm::vec4 bz = boxTranform[2];

	glm::vec4 center = boxTranform * (0.5f * (boxMax + boxMin));
	glm::vec4 localSize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec4 p = point - center;

	float px = glm::dot(p, bx);
	float py = glm::dot(p, by);
	float pz = glm::dot(p, bz);
	glm::vec4 delta = localSize - glm::abs(glm::vec4(px, py, pz, 0));

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


bool Collision::collide_PointvsAxisAlignedBox(const glm::vec4& point, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report)
{
	if (glm::any(glm::lessThan(point, boxMin)) || glm::any(glm::greaterThan(point, boxMax)))
		return false;
	else 
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(point);

			glm::vec4 v = point - 0.5f * (boxMax + boxMin);
			glm::vec4 delta = 0.5f * glm::abs(boxMax - boxMin) - glm::abs(v);

			if (delta.x < delta.y && delta.x < delta.z)
			{
				report->depths.push_back(delta.x);
				report->normal = v.x > 0.f ? glm::vec4(-1, 0, 0, 0) : glm::vec4(1, 0, 0, 0);
			}
			else if (delta.y < delta.x && delta.y < delta.z)
			{
				report->depths.push_back(delta.y);
				report->normal = v.y > 0.f ? glm::vec4(0, -1, 0, 0) : glm::vec4(0, 1, 0, 0);
			}
			else
			{
				report->depths.push_back(delta.z);
				report->normal = v.z > 0.f ? glm::vec4(0, 0, -1, 0) : glm::vec4(0, 0, 1, 0);
			}
		}

		return true;
	}
}
bool Collision::collide_AxisAlignedBoxvsPoint(const glm::vec4& point, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report)
{
	if (glm::any(glm::lessThan(point, boxMin)) || glm::any(glm::greaterThan(point, boxMax)))
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			glm::vec4 center = 0.5f * (boxMax + boxMin);
			glm::vec4 size = 0.5f * glm::abs(boxMax - boxMin);
			glm::vec4 v = point - center;
			glm::vec4 delta = size - glm::abs(v);

			if (delta.x < delta.y && delta.x < delta.z)
			{
				report->depths.push_back(delta.x);
				report->normal = v.x > 0.f ? glm::vec4(1, 0, 0, 0) : glm::vec4(-1, 0, 0, 0);
				report->points.push_back(glm::vec4(center.x + size.x, point.y, point.z, 1));
			}
			else if (delta.y < delta.x && delta.y < delta.z)
			{
				report->depths.push_back(delta.y);
				report->normal = v.y > 0.f ? glm::vec4(0, 1, 0, 0) : glm::vec4(0, -1, 0, 0);
				report->points.push_back(glm::vec4(point.x, center.y + size.y, point.z, 1));
			}
			else
			{
				report->depths.push_back(delta.z);
				report->normal = v.z > 0.f ? glm::vec4(0, 0, 1, 0) : glm::vec4(0, 0, -1, 0);
				report->points.push_back(glm::vec4(point.x, point.y, center.z + size.z, 1));
			}
		}

		return true;
	}
}


bool Collision::collide_PointvsSphere(const glm::vec4& point, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	glm::vec4 v = sphereCenter - point;
	float vv = glm::length2(v);
	if (vv < sphereRadius * sphereRadius)
	{
		if (report)
		{
			report->collision = true;
			report->points.push_back(point);
			report->depths.push_back(sphereRadius - glm::sqrt(vv));

			if (vv > COLLISION_EPSILON * COLLISION_EPSILON)
				report->normal = v / vv;
			else
				report->normal = glm::vec4(0, 1, 0, 0);
		}
		return true;
	}
	return false;
}
bool Collision::collide_SpherevsPoint(const glm::vec4& point, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	glm::vec4 v = sphereCenter - point;
	float vv = glm::length2(sphereCenter - point);
	if (vv < sphereRadius * sphereRadius)
	{
		if (report)
		{
			report->collision = true;
			report->depths.push_back(sphereRadius - glm::sqrt(vv));

			if (vv > COLLISION_EPSILON * COLLISION_EPSILON)
				report->normal = v / vv;
			else
				report->normal = glm::vec4(0, 1, 0, 0);

			report->points.push_back(sphereRadius + sphereRadius * report->normal);
		}
		return true;
	}
	return false;
}


bool Collision::collide_PointvsCapsule(const glm::vec4& point, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (capsule2 == capsule1) 
		return collide_PointvsSphere(point, capsule1, capsuleRadius, report);
	else if (capsuleRadius < COLLISION_EPSILON) 
		return collide_PointvsSegment(point, capsule1, capsule2, report);

	glm::vec4 closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, point);
	glm::vec4 v = point - closest;
	float vv = glm::dot(v, v);
	if (vv < capsuleRadius * capsuleRadius)
	{
		if (report)
		{
			report->collision = true;
			report->depths.push_back(capsuleRadius - glm::sqrt(vv));
			report->points.push_back(point);

			if (vv < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				glm::vec4 s = capsule2 - capsule1;
				report->normal = abs(s.x) > abs(s.z) ? glm::vec4(-s.y, s.x, 0, 0) : glm::vec4(0, -s.z, s.y, 0);
			}
			else
				report->normal = -v / vv;
		}
		return true;
	}
	return false;
}
bool Collision::collide_CapsulevsPoint(const glm::vec4& point, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (capsule2 == capsule1)
		return collide_SpherevsPoint(point, capsule1, capsuleRadius, report);
	else if (capsuleRadius < COLLISION_EPSILON)
		return collide_SegmentvsPoint(point, capsule1, capsule2, report);

	glm::vec4 closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, point);
	glm::vec4 v = point - closest;
	float vv = glm::dot(v, v);
	if (vv < capsuleRadius * capsuleRadius)
	{
		if (report)
		{
			report->collision = true;
			report->depths.push_back(capsuleRadius - glm::sqrt(vv));

			if (vv < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				glm::vec4 s = capsule2 - capsule1;
				report->normal = abs(s.x) > abs(s.z) ? glm::vec4(-s.y, s.x, 0, 0) : glm::vec4(0, -s.z, s.y, 0);
				report->normal = glm::normalize(report->normal);
			}
			else
				report->normal = v / vv;

			report->points.push_back(closest + capsuleRadius * report->normal);
		}
		return true;
	}
	return false;
}


bool Collision::collide_PointvsHull(const glm::vec4& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report)
{
	float dmax = std::numeric_limits<float>::min();
	unsigned int closestFace = 0;
	glm::vec4 p = glm::inverse(hullBase) * point;
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		float d = glm::dot(hullNormals[i], (glm::vec3)p - hullPoints[hullFaces[3 * i]]);

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
		report->normal = hullBase * glm::vec4(hullNormals[closestFace], 0.f);
	}
	return true;
}
bool Collision::collide_HullvsPoint(const glm::vec4& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report)
{
	float dmax = std::numeric_limits<float>::min();
	unsigned int closestFace = 0;
	glm::vec4 p = glm::inverse(hullBase) * point;
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		float d = glm::dot(hullNormals[i], (glm::vec3)p - hullPoints[hullFaces[3 * i]]);

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
		report->normal = hullBase * glm::vec4(hullNormals[closestFace], 0.f);

		const glm::vec3 t1 = glm::vec3(hullBase * glm::vec4(hullPoints[hullFaces[3 * closestFace]], 1.f));
		const glm::vec3 t2 = glm::vec3(hullBase * glm::vec4(hullPoints[hullFaces[3 * closestFace + 1]], 1.f));
		const glm::vec3 t3 = glm::vec3(hullBase * glm::vec4(hullPoints[hullFaces[3 * closestFace + 2]], 1.f));
		glm::vec3 u1 = t2 - t1;
		glm::vec3 u2 = t3 - t1;

		glm::vec4 u = p - report->normal * glm::dot(p, report->normal);
		glm::vec2 barry = CollisionUtils::getBarycentricCoordinates(u1, u2, (glm::vec3)u);

		report->points.push_back(glm::vec4(t1 + barry.x * u1 + barry.y + u2, 1.f));
	}
	return true;
}
//
