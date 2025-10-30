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

bool Collision::collide_CapsulevsCapsule(const vec4f& capsule1a, const vec4f& capsule1b, const float& capsule1Radius, const vec4f& capsule2a, const vec4f& capsule2b, 
	const float& capsule2Radius, CollisionReport* report)
{
	std::pair<vec4f, vec4f> closestPair = CollisionUtils::getSegmentsClosestSegment(capsule1a, capsule1b, capsule2a, capsule2b);
	vec4f delta = closestPair.first - closestPair.second;
	float radsum = capsule1Radius + capsule2Radius;
	float dd = vec4f::dot(delta, delta);
	if (dd > radsum * radsum)
		return false;

	if (report)
	{
		bool fallback = true;
		vec4f normal = delta;
		float d = std::sqrt(dd);
		if (dd < COLLISION_EPSILON * COLLISION_EPSILON)
			normal = vec4f(0, 1, 0, 0);
		else normal *= 1.f / d;

		if (0)//report->computeManifoldContacts)
		{
			vec4f s1 = capsule1b - capsule1a;
			vec4f s2 = capsule2b - capsule2a;
			float ss1 = s1.getNorm();
			float ss2 = s2.getNorm();
			s1 /= ss1;
			s2 /= ss2;
			float dots = std::abs(vec4f::dot(s1, s2));
			float dotn = std::abs(vec4f::dot(s1, normal));
			
			if (dots > 0.99f && dotn < 0.1f)
			{
				vec4f points[2][2];
				int overlap = 0;
				vec4f u1 = capsule2a - capsule1a;
				float dot = vec4f::dot(u1, s1);
				if (dot > 0.f && dot < ss1)
				{
					points[overlap][0] = capsule1a + dot * s1;
					points[overlap][1] = capsule2a;
					overlap++;
				}

				vec4f u2 = capsule2b - capsule1a;
				dot = vec4f::dot(u2, s1);
				if (dot > 0.f && dot < ss1)
				{
					points[overlap][0] = capsule1a + dot * s1;;
					points[overlap][1] = capsule2b;
					overlap++;
				}

				if (overlap < 2)
				{
					dot = vec4f::dot(-u1, s2);
					if (dot > 0.f && dot < ss2)
					{
						points[overlap][0] = capsule1a;
						points[overlap][1] = capsule2a + dot * s2;
						overlap++;
					}
				}
				if (overlap < 2)
				{
					dot = vec4f::dot(-u2, s2);
					if (dot > 0.f && dot < ss2)
					{
						points[overlap][0] = capsule2a;
						points[overlap][1] = capsule2a + dot * s2;
						overlap++;
					}
				}

				if (overlap == 2)
				{
					fallback = false;
					report->depths.push_back(radsum - (points[0][0], points[0][1]).getNorm());
					report->depths.push_back(radsum - (points[1][0], points[1][1]).getNorm());
					report->points.push_back(points[0][0] - capsule1Radius * normal);
					report->points.push_back(points[1][0] - capsule1Radius * normal);
				}
			}
		}

		if (fallback)
		{
			report->depths.push_back(radsum - d);
			report->points.push_back(closestPair.first - capsule1Radius * normal);
		}
		report->normal = normal;
		report->collision = true;
	}
	return true;
}

bool Collision::collide_CapsulevsTriangle(const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report)
{
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

