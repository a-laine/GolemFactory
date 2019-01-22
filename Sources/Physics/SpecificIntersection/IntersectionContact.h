#pragma once

#include <glm/glm.hpp>

namespace Intersection
{
	struct Contact
	{
		Contact();
		float time;
		glm::vec3 contactPoint;
		glm::vec3 normal;
		float distance;
	};
}
