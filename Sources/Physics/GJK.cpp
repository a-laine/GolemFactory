#include "GJK.h"
#include "SpecificCollision/CollisionPoint.h"


#include <iostream>

#define MAX_ITERATION 30000

//  Default
GJK::GJK() {}
//

//	Public functions
bool GJK::collide(const Shape& a, const Shape& b)
{
	glm::vec3 direction = glm::vec3(1, 0, 0);
	glm::vec3 S = a.GJKsupport(direction) - b.GJKsupport(-direction);
	std::vector<glm::vec3> simplex;
	simplex.reserve(4);
	simplex.push_back(S);
	direction = -direction;

	for(unsigned int i=0; i < MAX_ITERATION; i++)
	{
		S = a.GJKsupport(direction) - b.GJKsupport(-direction);
		if (glm::dot(S, direction) < 0)
			return false;

		simplex.push_back(S);

	}

	std::cout << "GJK : error : no solution found after maximum iteration (" << MAX_ITERATION << ")" << std::endl;
	return false;
}
//

//
bool GJK::doSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction)
{

}
bool GJK::containOrigin(std::vector<glm::vec3>& simplex)
{
	if (simplex.size() == 1)
		return simplex[0] == glm::vec3(0, 0, 0);
	else if (simplex.size() == 2)
		return Collision::collide_PointvsSegment(glm::vec3(0, 0, 0), simplex[0], simplex[1]);
	else if (simplex.size() == 3)
		return Collision::collide_PointvsTriangle(glm::vec3(0, 0, 0), simplex[0], simplex[1], simplex[2]);
	else if (simplex.size() == 4)
	{
		glm::vec3 n1 = glm::cross(simplex[1] - simplex[0], simplex[2] - simplex[0]);
		if (glm::dot(n1, simplex[3] - simplex[0]) > 0) n1 *= -1.f;
		glm::vec3 n2 = glm::cross(simplex[1] - simplex[0], simplex[3] - simplex[0]);
		if (glm::dot(n2, simplex[2] - simplex[0]) > 0) n2 *= -1.f;
		glm::vec3 n3 = glm::cross(simplex[3] - simplex[2], simplex[1] - simplex[2]);
		if (glm::dot(n3, simplex[0] - simplex[2]) > 0) n3 *= -1.f;
		glm::vec3 n4 = glm::cross(simplex[3] - simplex[2], simplex[1] - simplex[2]);
		if (glm::dot(n4, simplex[1] - simplex[2]) > 0) n4 *= -1.f;

		if (glm::dot(n1, -simplex[0])) return true;
		else if (glm::dot(n2, -simplex[0])) return true;
		else if (glm::dot(n3, -simplex[2])) return true;
		else if (glm::dot(n4, -simplex[2])) return true;
		else return false;
	}
	else 
	{
		std::cout << "GJK : error : simplex check not possible in normal case (dimension too high or too low)" << std::endl;
		return true;
	}
}
//