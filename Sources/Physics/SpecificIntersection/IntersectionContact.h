#pragma once

#include <glm/glm.hpp>

namespace Intersection
{
	struct Contact
	{
		Contact();
		Contact(const glm::vec3& A, const glm::vec3& B, const glm::vec3& nA, const glm::vec3& nB);
		Contact& swap();

		glm::vec3 contactPointA;
		glm::vec3 contactPointB;
		glm::vec3 normalA;
		glm::vec3 normalB;
	};
}
