#include "CollisionUtils.h"
#include <Physics/Collision.h>


//	Specialized functions : capsule
/*bool Collision2::collide_CapsulevsCapsule(const glm::vec4& capsule1a, const glm::vec4& capsule1b, const float& capsule1Radius, const glm::vec4& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	if (capsule1a == capsule1b) return collide_SpherevsCapsule(capsule1a, capsule1Radius, capsule2a, capsule2b, capsule2Radius);
	else if (capsule2a == capsule2b) return collide_SpherevsCapsule(capsule2a, capsule2Radius, capsule1a, capsule1b, capsule1Radius);

	std::pair<glm::vec4, glm::vec4> p = CollisionUtils::getSegmentsClosestSegment(capsule1a, capsule1b, capsule2a, capsule2b);
	return glm::dot(p.first - p.second, p.first - p.second) <= (capsule1Radius + capsule2Radius) * (capsule1Radius + capsule2Radius);
	
}*/

bool Collision::collide_CapsulevsTriangle(const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report)
{
	//Capsule capsule(capsule1, capsule2, capsuleRadius);
	//Triangle triangle(triangle1, triangle2, triangle3);
	//return _GJKCollision(&capsule, &triangle, report);

	std::pair<vec4f, vec4f> closestPair = CollisionUtils::getClosestPair(capsule1, capsule2, triangle1, triangle2, triangle3);
	vec4f delta = closestPair.first - closestPair.second;
	delta.w = 0;
	float dd = vec4f::dot(delta, delta);
	if (dd > capsuleRadius * capsuleRadius)
		return false;

	if (report)
	{
		vec4f t12 = triangle2 - triangle1;
		vec4f t13 = triangle3 - triangle1;
		vec4f tnormal = vec4f::cross(t12, t13);
		tnormal.w = 0.f;
		tnormal.normalize();

		float d = std::sqrt(dd);
		if (d < COLLISION_EPSILON)
		{
			report->collision = true;
			report->normal = tnormal;
			float dot1 = vec4f::dot(triangle1 - capsule1, tnormal);
			float dot2 = vec4f::dot(triangle1 - capsule2, tnormal);
			if (dot1 < dot2)
			{
				report->points.push_back(capsule1 - tnormal * capsuleRadius);
				report->depths.push_back(std::abs(dot1) + capsuleRadius);
			}
			else
			{
				report->points.push_back(capsule2 - tnormal * capsuleRadius);
				report->depths.push_back(std::abs(dot2) + capsuleRadius);
			}
		}
		/*else if (report->computeManifoldContacts)
		{
			report->collision = true;

		}*/
		else
		{
			report->collision = true;
			report->normal = (1.f / d) * delta;
			report->depths.push_back(capsuleRadius - d);
			report->points.push_back(closestPair.first - capsuleRadius * report->normal);
		}
	}
	return true;
}
//

