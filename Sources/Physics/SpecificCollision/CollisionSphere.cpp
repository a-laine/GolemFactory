#include "CollisionUtils.h"
#include <Physics/Collision.h>

//#include <glm/gtx/norm.hpp>
//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/component_wise.hpp>

//	Specialized functions : sphere
bool Collision::collide_SpherevsSphere(const glm::vec4& sphere1Center, const float& sphere1Radius, const glm::vec4& sphere2Center, const float& sphere2Radius, CollisionReport* report)
{
	glm::vec4 v = sphere2Center - sphere1Center;
	float radiusSum = sphere1Radius + sphere2Radius;
	float vv = glm::length2(v);

	if (vv > radiusSum * radiusSum)
		return false;
	else
	{
		if (report)
		{
			float length = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(radiusSum - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
				report->normal = glm::vec4(0, 1, 0, 0);

			report->points.push_back(sphere1Center + sphere1Radius * report->normal);
		}
		return true;
	}
}

bool Collision::collide_SpherevsCapsule(const glm::vec4& sphereCenter, const float& sphereRadius, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	glm::vec4 closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, sphereCenter);
	float radiusSum = sphereRadius + capsuleRadius;
	glm::vec4 v = closest - sphereCenter; v.w = 0.f;
	float vv = glm::length2(v);

	if (vv > radiusSum * radiusSum)
		return false;
	else
	{
		if (report)
		{
			float length = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(radiusSum - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
			{
				glm::vec4 s = capsule1 - capsule2;
				report->normal = abs(s.x) > abs(s.z) ? glm::vec4(-s.y, s.x, 0.f, 0.f) : glm::vec4(0.f, -s.z, s.y, 0.f);
				report->normal = glm::normalize(report->normal);
			}
			report->points.push_back(sphereCenter + sphereRadius * report->normal);
		}
		return true;
	}
}
bool Collision::collide_CapsulevsSphere(const glm::vec4& sphereCenter, const float& sphereRadius, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	glm::vec4 closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, sphereCenter);
	float radiusSum = sphereRadius + capsuleRadius;
	glm::vec4 v = sphereCenter - closest;
	float vv = glm::length2(v);

	if (vv > radiusSum * radiusSum)
		return false;
	else
	{
		if (report)
		{
			float length = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(radiusSum - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
			{
				glm::vec4 s = capsule1 - capsule2;
				report->normal = abs(s.x) > abs(s.z) ? glm::vec4(-s.y, s.x, 0.f, 0.f) : glm::vec4(0.f, -s.z, s.y, 0.f);
				report->normal = glm::normalize(report->normal);
			}
			report->points.push_back(closest + capsuleRadius * report->normal);
		}
		return true;
	}
}

bool Collision::collide_AxisAlignedBoxvsSphere(const glm::vec4& boxMin, const glm::vec4& boxMax, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (Collision::collide_AxisAlignedBoxvsPoint(sphereCenter, boxMin, boxMax, report))
	{
		if (report)
			report->depths.back() += sphereRadius;
		return true;
	}

	glm::vec4 center = 0.5f * (boxMax + boxMin);
	glm::vec4 size = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec4 p = sphereCenter - center;
	glm::vec4 closest = center + glm::clamp(p, -size, size);
	glm::vec4 v = sphereCenter - closest;
	float vv = glm::length2(v);

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float length = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(sphereRadius - length);
			report->points.push_back(closest);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
				report->normal = glm::normalize(closest - center);
		}
		return true;
	}
}
bool Collision::collide_SpherevsAxisAlignedBox(const glm::vec4& boxMin, const glm::vec4& boxMax, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (Collision::collide_PointvsAxisAlignedBox(sphereCenter, boxMin, boxMax, report))
	{
		if (report)
			report->depths.back() += sphereRadius;
		return true;
	}

	glm::vec4 center = 0.5f * (boxMax + boxMin);
	glm::vec4 size = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec4 p = sphereCenter - center;
	glm::vec4 closest = center + glm::clamp(p, -size, size);
	glm::vec4 v = closest - sphereCenter;
	float vv = glm::length2(v);

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float length = glm::sqrt(vv);
			report->collision = true;
			report->depths.push_back(sphereRadius - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
				report->normal = glm::normalize(center - closest);
			report->points.push_back(sphereCenter + sphereRadius * report->normal);
		}
		return true;
	}
}


/*bool Collision::collide_SpherevsHull(const glm::vec3& sphereCenter, const float& sphereRadius, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report)
{
	struct Triangle
	{
		Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) : p1(a), p2(b), p3(c) {};
		glm::vec3 p1, p2, p3;
	};

	//	search for all faces front to sphere center
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * glm::vec4(sphereCenter, 1.f));
	std::vector<Triangle> frontFaces;
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		if (glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]) >= 0)
			frontFaces.push_back(Triangle(hullPoints[hullFaces[3 * i]], hullPoints[hullFaces[3 * i + 1]], hullPoints[hullFaces[3 * i + 2]]));
	}

	if(frontFaces.empty())
		return true;

	//	test if each front faces collide
	for (unsigned int i = 0; i < frontFaces.size(); i++)
	{
		if (collide_TrianglevsSphere(frontFaces[i].p1, frontFaces[i].p2, frontFaces[i].p3, p, sphereRadius))
			return true;
	}
	return false;
}*/
//