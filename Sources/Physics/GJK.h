#pragma once
#include "Shapes/Shape.h"
#include "SpecificIntersection/IntersectionContact.h"

#include <vector>

class GJK
{
	public:
		//	Public functions
		static bool collide(const Shape& a, const Shape& b, std::vector<std::pair<glm::vec3, glm::vec3>>* shapePair = nullptr);
		static Intersection::Contact intersect(const Shape& a, const Shape& b);
		//

	protected:
		//	Protected functions
		static void prepareSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction, std::vector<std::pair<glm::vec3, glm::vec3>>& simplexPoints);
		static bool containOrigin(std::vector<glm::vec3>& simplex);
		//
};



