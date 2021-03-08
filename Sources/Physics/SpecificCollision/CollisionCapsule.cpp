#include "CollisionCapsule.h"
#include "CollisionSphere.h"
#include "CollisionUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>


//	Specialized functions : capsule
bool Collision::collide_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	if (capsule1a == capsule1b) return collide_SpherevsCapsule(capsule1a, capsule1Radius, capsule2a, capsule2b, capsule2Radius);
	else if (capsule2a == capsule2b) return collide_SpherevsCapsule(capsule2a, capsule2Radius, capsule1a, capsule1b, capsule1Radius);

	std::pair<glm::vec3, glm::vec3> p = getSegmentsClosestSegment(capsule1a, capsule1b, capsule2a, capsule2b);
	return glm::dot(p.first - p.second, p.first - p.second) <= (capsule1Radius + capsule2Radius) * (capsule1Radius + capsule2Radius);
	
}
//

