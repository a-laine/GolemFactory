#include "CollisionCapsule.h"
#include "CollisionSphere.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>


//	Specialized functions : capsule
bool Collision::collide_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	glm::vec3 s1 = capsule1b - capsule1a;
	glm::vec3 s2 = capsule2b - capsule2a;
	glm::vec3 u1 = glm::normalize(s1);
	glm::vec3 u2 = glm::normalize(s2);
	glm::vec3 n = glm::cross(u1, u2);

	if (glm::dot(n, n) == 0.f)	// parallel or one segment is a point
	{
		if (u1 == glm::vec3(0.f))
			return collide_SpherevsCapsule(capsule1b, capsule1Radius, capsule2a, capsule2b, capsule2Radius);
		else if (u2 == glm::vec3(0.f))
			return collide_SpherevsCapsule(capsule2b, capsule2Radius, capsule1a, capsule1b, capsule1Radius);
		else // segment are parallel
		{
			glm::vec3 u3 = capsule1a - capsule2a;
			glm::vec3 d = u3 - u1 * std::abs(glm::dot(u3, u1));
			return glm::length(d) <= capsule1Radius + capsule2Radius;
		}
	}
	else
	{
		float t1 = std::min(glm::length(s1), std::max(0.f, glm::determinant(glm::mat3(capsule1a - capsule2a, u2, n)) / glm::dot(n, n)));
		float t2 = std::min(glm::length(s2), std::max(0.f, glm::determinant(glm::mat3(capsule1a - capsule2a, u1, n)) / glm::dot(n, n)));
		glm::vec3 d = capsule2a + u2*t2 - (capsule1a + u1*t1);
		return glm::length(d) <= capsule1Radius + capsule2Radius;
	}
}
//

