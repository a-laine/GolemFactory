#pragma once
#include "Shapes/Shape.h"

#include <vector>

class GJK
{
	public:
		//  Default
		GJK();
		//

		//	Public functions
		bool collide(const Shape& a, const Shape& b);
		//

	protected:
		//	Protected functions
		bool doSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction);
		bool containOrigin(std::vector<glm::vec3>& simplex);
		//
};



