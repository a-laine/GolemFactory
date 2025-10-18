#include "CollisionUtils.h"
#include <Physics/Collision.h>

//#include "CollisionAxisAlignedBox.h"


//#include <Physics/SpecificIntersection/IntersectionSegment.h>

//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/component_wise.hpp>
//#include <glm/gtx/norm.hpp>


//	Specialized functions : axis aligned box
/*bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max)
{
	if (glm::any(glm::greaterThan(box1Min, box2Max)) || glm::any(glm::greaterThan(box2Min, box1Max)))
		return false;
	return true;
}*/
bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max, CollisionReport* report)
{
	if (vec4b::any(vec4f::greaterThan(box1Min, box2Max)) || vec4b::any(vec4f::greaterThan(box2Min, box1Max)))
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			vec4f center1 = 0.5f * (box1Max + box1Min);
			vec4f center2 = 0.5f * (box2Max + box2Min);
			vec4f size1 = 0.5f * (box1Max - box1Min);
			vec4f size2 = 0.5f * (box2Max - box2Min);
			vec4f v = center1 - center2;
			vec4f delta = vec4f::abs(v) - size1 - size2;

			report->depths.push_back(std::numeric_limits<float>::max());
			if (delta.x < 0.f && -delta.x < report->depths.back())
			{
				report->depths.back() = delta.x;
				report->normal = vec4f(v.x > 0.f ? 1 : -1, 0, 0, 1);
			}
			if (delta.y < 0.f && -delta.y < report->depths.back())
			{
				report->depths.back() = delta.y;
				report->normal = vec4f(0, v.y > 0.f ? 1 : -1, 0, 1);
			}
			if (delta.z < 0.f && -delta.z < report->depths.back())
			{
				report->depths.back() = delta.z;
				report->normal = vec4f(0, 0, v.z > 0.f ? 1 : -1, 1);
			}
			report->points.push_back(center1 + report->normal * vec4f::dot(report->normal, size1));
		}
		return true;
	}
}

bool Collision::raycast_AxisAlignedBox(const vec4f& rayOrigin, const vec4f& rayDirection, const vec4f& boxMin, const vec4f& boxMax, RaycastReport* report)
{
	// test if already in box
	if (vec4b::all(vec4f::greaterThan(rayOrigin, boxMin)) && vec4b::all(vec4f::lessThan(rayOrigin, boxMax)))
	{
		if (report)
		{
			report->m_distance = 0.f;
			report->m_intersection = rayOrigin;
			report->m_intersection.w = 1;

			vec4f center = 0.5f * (boxMax + boxMin);
			vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
			vec4f v = rayOrigin - center;
			vec4f delta = size - vec4f::abs(v);

			if (delta.x < delta.y && delta.x < delta.z)
				report->m_normal = v.x > 0.f ? vec4f(1, 0, 0, 0) : vec4f(-1, 0, 0, 0);
			else if (delta.y < delta.x && delta.y < delta.z)
				report->m_normal = v.y > 0.f ? vec4f(0, 1, 0, 0) : vec4f(0, -1, 0, 0);
			else
				report->m_normal = v.z > 0.f ? vec4f(0, 0, 1, 0) : vec4f(0, 0, -1, 0);
		}
		return true;
	}

	//vec4f s = segment2 - rayOrigin;
	//float rayLength = s.getNorm();
	if (rayDirection.w < COLLISION_EPSILON)
		return false;

	// ray box regular
	//vec4f rayDirection = s * (1.f / rayLength);
	vec4f rayInvDirection = vec4f::zero;
	rayInvDirection.x = std::abs(rayDirection.x) > COLLISION_EPSILON * COLLISION_EPSILON ? 1.f / rayDirection.x : 1E12f;
	rayInvDirection.y = std::abs(rayDirection.y) > COLLISION_EPSILON * COLLISION_EPSILON ? 1.f / rayDirection.y : 1E12f;
	rayInvDirection.z = std::abs(rayDirection.z) > COLLISION_EPSILON * COLLISION_EPSILON ? 1.f / rayDirection.z : 1E12f;

	vec4f t0 = (boxMax - rayOrigin) * rayInvDirection;
	vec4f t1 = (boxMin - rayOrigin) * rayInvDirection;
	vec4f tmin = vec4f::min(t0, t1);
	vec4f tmax = vec4f::max(t0, t1);
	float distanceB = std::min(std::min(tmax.x, tmax.y), tmax.z);
	float distanceA = std::max(std::max(tmin.x, tmin.y), tmin.z);
	if (distanceB < distanceA || distanceA > rayDirection.w)
		return false;

	if (report)
	{
		report->m_distance = distanceA;
		report->m_intersection = rayOrigin + distanceA * rayDirection;
		report->m_intersection.w = 1;

		vec4f center = 0.5f * (boxMax + boxMin);
		vec4f size = 0.5f * vec4f::abs(boxMax - boxMin);
		vec4f v = report->m_intersection - center;
		vec4f delta = size - vec4f::abs(v);

		if (delta.x < delta.y && delta.x < delta.z)
			report->m_normal = v.x > 0.f ? vec4f(1, 0, 0, 0) : vec4f(-1, 0, 0, 0);
		else if (delta.y < delta.x && delta.y < delta.z)
			report->m_normal = v.y > 0.f ? vec4f(0, 1, 0, 0) : vec4f(0, -1, 0, 0);
		else
			report->m_normal = v.z > 0.f ? vec4f(0, 0, 1, 0) : vec4f(0, 0, -1, 0);
	}
	return true;
}
//