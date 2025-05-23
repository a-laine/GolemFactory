#include "CollisionUtils.h"
#include <Physics/Collision.h>

//#include <glm/gtx/norm.hpp>
//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/component_wise.hpp>

//	Specialized functions : sphere
bool Collision::collide_SpherevsSphere(const vec4f& sphere1Center, const float& sphere1Radius, const vec4f& sphere2Center, const float& sphere2Radius, CollisionReport* report)
{
	vec4f v = sphere2Center - sphere1Center;
	float radiusSum = sphere1Radius + sphere2Radius;
	float vv = v.getNorm2();

	if (vv > radiusSum * radiusSum)
		return false;
	else
	{
		if (report)
		{
			float length = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(radiusSum - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
				report->normal = vec4f(0, 1, 0, 0);

			report->points.push_back(sphere1Center + sphere1Radius * report->normal);
		}
		return true;
	}
}

bool Collision::collide_SpherevsCapsule(const vec4f& sphereCenter, const float& sphereRadius, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	vec4f closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, sphereCenter);
	float radiusSum = sphereRadius + capsuleRadius;
	vec4f v = closest - sphereCenter; v.w = 0.f;
	float vv = v.getNorm2();

	if (vv > radiusSum * radiusSum)
		return false;
	else
	{
		if (report)
		{
			float length = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(radiusSum - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
			{
				vec4f s = capsule1 - capsule2;
				report->normal = std::abs(s.x) > std::abs(s.z) ? vec4f(-s.y, s.x, 0.f, 0.f) : vec4f(0.f, -s.z, s.y, 0.f);
				report->normal.normalize();
			}
			report->points.push_back(sphereCenter + sphereRadius * report->normal);
		}
		return true;
	}
}
bool Collision::collide_CapsulevsSphere(const vec4f& sphereCenter, const float& sphereRadius, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report)
{
	vec4f closest = CollisionUtils::getSegmentClosestPoint(capsule1, capsule2, sphereCenter);
	float radiusSum = sphereRadius + capsuleRadius;
	vec4f v = sphereCenter - closest;
	float vv = v.getNorm2();

	if (vv > radiusSum * radiusSum)
		return false;
	else
	{
		if (report)
		{
			float length = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(radiusSum - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
			{
				vec4f s = capsule1 - capsule2;
				report->normal = std::abs(s.x) > std::abs(s.z) ? vec4f(-s.y, s.x, 0.f, 0.f) : vec4f(0.f, -s.z, s.y, 0.f);
				report->normal.normalize();
			}
			report->points.push_back(closest + capsuleRadius * report->normal);
		}
		return true;
	}
}

bool Collision::collide_SpherevsOrientedBox(const vec4f& sphereCenter, const float& sphereRadius, const mat4f& boxTranform, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report)
{
	vec4f bx = boxTranform[0];
	vec4f by = boxTranform[1];
	vec4f bz = boxTranform[2];

	vec4f center = boxTranform * (0.5f * (boxMax + boxMin));
	vec4f halfSize = 0.5f * vec4f::abs(boxMax - boxMin);
	vec4f p = sphereCenter - center;

	float px = clamp(vec4f::dot(p, bx), -halfSize.x, halfSize.x);
	float py = clamp(vec4f::dot(p, by), -halfSize.y, halfSize.y);
	float pz = clamp(vec4f::dot(p, bz), -halfSize.z, halfSize.z);
	vec4f closest = center + bx * px + by * py + bz * pz;
	vec4f delta = sphereCenter - closest;
	float distance2 = delta.getNorm2();
	
	if (distance2 > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			if (distance2 < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				delta = halfSize - vec4f::abs(vec4f(px, py, pz, 0));
				if (delta.x < delta.y && delta.x < delta.z)
				{
					report->depths.push_back(sphereRadius + delta.x);
					report->normal = px > 0.f ? -bx : bx;
				}
				else if (delta.y < delta.x&& delta.y < delta.z)
				{
					report->depths.push_back(sphereRadius + delta.y);
					report->normal = py > 0.f ? -by : by;
				}
				else
				{
					report->depths.push_back(sphereRadius + delta.z);
					report->normal = pz > 0.f ? -bz : bz;
				}
				report->points.push_back(sphereCenter - sphereRadius * report->normal);
			}
			else
			{
				float distance = sqrt(distance2);
				report->normal = (-1.f / distance) * delta;
				report->depths.push_back(sphereRadius - distance);
				report->points.push_back(sphereCenter + sphereRadius * report->normal);
			}
		}
		return true;
	}
}
bool Collision::collide_OrientedBoxvsSphere(const vec4f& sphereCenter, const float& sphereRadius, const mat4f& boxTranform, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report)
{
	vec4f bx = boxTranform[0];
	vec4f by = boxTranform[1];
	vec4f bz = boxTranform[2];

	vec4f center = boxTranform * (0.5f * (boxMax + boxMin));
	vec4f localHalfSize = 0.5f * vec4f::abs(boxMax - boxMin);
	bx *= std::abs(localHalfSize.x);
	by *= std::abs(localHalfSize.y);
	bz *= std::abs(localHalfSize.z);

	vec4f p = sphereCenter - center;

	float px = clamp(vec4f::dot(p, bx), -1.f, 1.f);
	float py = clamp(vec4f::dot(p, by), -1.f, 1.f);
	float pz = clamp(vec4f::dot(p, bz), -1.f, 1.f);
	vec4f closest = center + bx * px + by * py + bz * pz;
	vec4f delta = sphereCenter - closest;
	float distance2 = delta.getNorm2();

	if (distance2 > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			if (distance2 < COLLISION_EPSILON * COLLISION_EPSILON)
			{
				vec4f halfSize = vec4f(bx.getNorm(), by.getNorm(), bz.getNorm(), 0.f);
				delta = halfSize - vec4f::abs(vec4f(px, py, pz, 0));
				if (delta.x < delta.y && delta.x < delta.z)
				{
					report->depths.push_back(sphereRadius + delta.x);
					report->normal = px > 0.f ? bx : -bx;
					report->normal.normalize();
					report->points.push_back(center - halfSize.x * report->normal);
				}
				else if (delta.y < delta.x&& delta.y < delta.z)
				{
					report->depths.push_back(sphereRadius + delta.y);
					report->normal = py > 0.f ? by : -by;
					report->normal.normalize();
					report->points.push_back(center - halfSize.y * report->normal);
				}
				else
				{
					report->depths.push_back(sphereRadius + delta.z);
					report->normal = pz > 0.f ? bz : -bz;
					report->normal.normalize();
					report->points.push_back(center - halfSize.z * report->normal);
				}
			}
			else
			{
				float distance = sqrt(distance2);
				report->normal = (1.f / distance) * delta;
				report->depths.push_back(sphereRadius - distance);
				report->points.push_back(closest);
			}
		}
		return true;
	}
}

bool Collision::collide_AxisAlignedBoxvsSphere(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (Collision::collide_AxisAlignedBoxvsPoint(sphereCenter, boxMin, boxMax, report))
	{
		if (report)
			report->depths.back() += sphereRadius;
		return true;
	}

	vec4f center = 0.5f * (boxMax + boxMin);
	vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
	vec4f p = sphereCenter - center;
	vec4f closest = center + vec4f::clamp(p, -size, size);
	vec4f v = sphereCenter - closest;
	float vv = v.getNorm2();

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float length = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(sphereRadius - length);
			report->points.push_back(closest);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
				report->normal = (closest - center).getNormal();
		}
		return true;
	}
}
bool Collision::collide_SpherevsAxisAlignedBox(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report)
{
	if (Collision::collide_PointvsAxisAlignedBox(sphereCenter, boxMin, boxMax, report))
	{
		if (report)
			report->depths.back() += sphereRadius;
		return true;
	}

	vec4f center = 0.5f * (boxMax + boxMin);
	vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
	vec4f p = sphereCenter - center;
	vec4f closest = center + vec4f::clamp(p, -size, size);
	vec4f v = closest - sphereCenter;
	float vv = v.getNorm2();

	if (vv > sphereRadius * sphereRadius)
		return false;
	else
	{
		if (report)
		{
			float length = std::sqrt(vv);
			report->collision = true;
			report->depths.push_back(sphereRadius - length);

			if (length > COLLISION_EPSILON)
				report->normal = v / length;
			else
				report->normal = (center - closest).getNormal();
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
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * vec4f(sphereCenter, 1.f));
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