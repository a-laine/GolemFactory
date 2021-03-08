#include "CollisionPoint.h"
#include "CollisionUtils.h"

#include <Physics/SpecificIntersection/IntersectionPoint.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>


//	Specialized functions : point
bool Collision::collide_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2, CollisionReport* report)
{
	bool collide = point1 == point2;
	if (collide && report)
	{
		report->collision = true;
		report->contactPoint1 = point1;
		report->contactPoint2 = point2;
		report->normal1 = glm::vec3(0, 1, 0);
		report->normal2 = glm::vec3(0, -1, 0);
	}
	return collide;
}
bool Collision::collide_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2, CollisionReport* report)
{
	if (segment1 == segment2) 
		return collide_PointvsPoint(point, segment1, report);
	else
	{
		glm::vec3 closest = getSegmentClosestPoint(segment1, segment2, point);
		glm::vec3 u = point - closest;
		bool collide = glm::length2(u) < (COLLISION_EPSILON * COLLISION_EPSILON);

		if (collide && report)
		{
			report->collision = true;
			report->contactPoint1 = point;
			report->contactPoint2 = closest;
			report->normal2 = glm::normalize(point - closest);
			report->normal1 = -report->normal2;
		}

		return collide;
	}
}
bool Collision::collide_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report)
{
	//	check if point is coplanar to triangle
	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(u1, u2);
	glm::vec3 p = point - triangle1;

	if (glm::length2(n) <= COLLISION_EPSILON) // flat triangle
	{
		glm::vec3 u3 = triangle3 - triangle2;
		float d1 = glm::length2(u1);
		float d2 = glm::length2(u2);
		float d3 = glm::length2(u3);

		if (d1 >= d2 && d1 >= d3) return collide_PointvsSegment(point, triangle1, triangle2, report);
		else if (d2 >= d1 && d2 >= d3) return collide_PointvsSegment(point, triangle1, triangle3, report);
		else return collide_PointvsSegment(point, triangle3, triangle2, report);
	}
	else if (std::abs(glm::dot(p, glm::normalize(n))) <= COLLISION_EPSILON) // close enough to triangle plane
	{
		//	checking barycentric coordinates
		glm::vec3 u = p - n * glm::dot(p, n);
		glm::vec2 barry = getBarycentricCoordinates(u1, u2, u);
		bool collision = !(barry.x < -COLLISION_EPSILON || barry.y < -COLLISION_EPSILON || barry.x + barry.y > 1.f + COLLISION_EPSILON);

		if (collision && report)
		{
			report->collision = collision;
			report->contactPoint1 = point;
			report->contactPoint2 = triangle1 + u;
			report->normal1 = -n;
			report->normal2 = n;
		}

		return collision;
	}
	else return false;
}
bool Collision::collide_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, CollisionReport* report)
{
	glm::vec3 bx = glm::vec3(boxTranform[0]);
	glm::vec3 by = glm::vec3(boxTranform[1]);
	glm::vec3 bz = glm::vec3(boxTranform[2]);

	glm::vec3 size = boxMax - boxMin;
	glm::vec3 localCenter = 0.5f * (boxMax + boxMin);
	glm::vec3 center = glm::vec3(boxTranform * glm::vec4(localCenter, 0.f));

	glm::vec3 p = point - center;

	float px = glm::dot(p, bx);
	float py = glm::dot(p, by);
	float pz = glm::dot(p, bz);

	if (std::abs(px) > size.x || std::abs(py) > size.y || std::abs(pz) > size.z)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;
			report->contactPoint1 = point;

		}

		return true;
	}

}
bool Collision::collide_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax, CollisionReport* report)
{
	if (point.x < boxMin.x || point.y < boxMin.y || point.z < boxMin.z) return false;
	else if (point.x > boxMax.x || point.y > boxMax.y || point.z > boxMax.z) return false;
	else 
	{
		if (report)
		{
			// init
			glm::vec3 bcenter = 0.5f * (boxMax + boxMin);
			glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
			glm::vec3 p = point - bcenter;
			glm::vec3 n = glm::vec3(0.f);

			const float dx = bsize.x - std::abs(p.x);
			const float dy = bsize.y - std::abs(p.y);
			const float dz = bsize.z - std::abs(p.z);

			// inside point is closer to an x face
			if (dx <= dy && dx <= dz)
			{
				if (p.x > 0)
				{
					n = glm::vec3(1, 0, 0);
					p.x = bsize.x;
				}
				else
				{
					n = glm::vec3(-1, 0, 0);
					p.x = -bsize.x;
				}
			}

			// inside point is closer to a y face
			else if (dy <= dx && dy <= dz)
			{
				if (p.y > 0)
				{
					n = glm::vec3(0, 1, 0);
					p.y = bsize.y;
				}
				else
				{
					n = glm::vec3(0, -1, 0);
					p.y = -bsize.y;
				}
			}

			// inside point is closer to a z face
			else
			{
				if (p.z > 0)
				{
					n = glm::vec3(0, 0, 1);
					p.z = bsize.z;
				}
				else
				{
					n = glm::vec3(0, 0, -1);
					p.z = -bsize.z;
				}
			}

			// report
			report->collision = true;
			report->contactPoint1 = point;
			report->contactPoint2 = p + bcenter;
			report->normal1 = glm::normalize(report->contactPoint2 - point);
			report->normal2 = n;
		}

		return true;
	}
}
bool Collision::collide_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	glm::vec3 v = sphereCenter - point;
	float vv = glm::dot(v, v);

	if (vv < sphereRadius * sphereRadius)
	{
		if (report)
		{
			if (vv > COLLISION_EPSILON * COLLISION_EPSILON)
				report->normal1 = v / vv;
			else
				report->normal1 = glm::vec3(0, 1, 0);

			report->collision = true;
			report->contactPoint1 = point;
			report->contactPoint2 = sphereCenter + sphereRadius * report->normal1;
			report->normal2 = -report->normal1;
		}
		return true;
	}
	return false;
}
bool Collision::collide_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	if (capsule2 == capsule1) 
		return collide_PointvsSphere(point, capsule1, capsuleRadius, report);
	else if (capsuleRadius < COLLISION_EPSILON) 
		return collide_PointvsSegment(point, capsule1, capsule2, report);

	glm::vec3 closest = getSegmentClosestPoint(capsule1, capsule2, point);
	glm::vec3 v = point - closest;
	float vv = glm::dot(v, v);
	if (vv < capsuleRadius * capsuleRadius)
	{
		if (report)
		{
			glm::vec3 n;
			if (vv < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				glm::vec3 s = capsule2 - capsule1;
				if (std::abs(s.y) > COLLISION_EPSILON)
					n = glm::cross(s, glm::vec3(0, 1, 0));
				else
					n = glm::cross(s, glm::vec3(1, 0, 0));
				glm::normalize(n);
			}
			else
				n = v / vv;

			report->collision = true;
			report->contactPoint1 = point;
			report->contactPoint2 = closest + capsuleRadius * n;
			report->normal1 = -n;
			report->normal2 = n;
		}
		return true;
	}
	return false;
}
bool Collision::collide_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report)
{
	float dmax = std::numeric_limits<float>::min();
	unsigned short closestFace = 0;
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * glm::vec4(point, 1.f));
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		float d = glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]);

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
		const glm::vec3 t1 = hullPoints[hullFaces[3 * closestFace]];
		const glm::vec3 t2 = hullPoints[hullFaces[3 * closestFace + 1]];
		const glm::vec3 t3 = hullPoints[hullFaces[3 * closestFace + 2]];
		Intersection::Contact result = Intersection::intersect_PointvsTriangle(point, t1, t2, t3);

		report->collision = true;
		report->contactPoint1 = point;
		report->contactPoint2 = result.contactPointB;
		report->normal1 = glm::normalize(report->contactPoint2 - point);
		report->normal2 = report->normal1;
	}
	return true;
}
//
