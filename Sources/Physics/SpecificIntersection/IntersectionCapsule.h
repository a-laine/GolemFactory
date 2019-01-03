#pragma once


#include "../Shape.h"

namespace Intersection
{
	//	Specialized functions : Capsule
	Result intersect_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius);
};