#pragma once
#include "Shapes/Shape.h"

#include <vector>

class GJK
{
	public:
		//	Public functions
		static bool collide(const Shape& a, const Shape& b);
		//

	protected:
		//	Protected functions
		static void prepareSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction);
		static bool containOrigin(std::vector<glm::vec3>& simplex);
		//
};



