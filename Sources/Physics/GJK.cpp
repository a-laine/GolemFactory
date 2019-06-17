#include "GJK.h"
#include "SpecificCollision/CollisionPoint.h"


#include <iostream>

#define MAX_ITERATION 20

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
		if (containOrigin(simplex))
			return true;
		prepareSimplex(simplex, direction);
	}

	std::cout << "GJK : error : no solution found after maximum iteration (" << MAX_ITERATION << ")" << std::endl;
	return false;
}
//

//
void GJK::prepareSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction)
{
	switch (simplex.size())
	{
		case 1:
			direction = -simplex[0];
			break;

		case 2:
			if (glm::dot(simplex[1] - simplex[0], -simplex[0]) > 0)
			{
				direction = glm::cross(glm::cross(simplex[1] - simplex[0], -simplex[0]), simplex[1] - simplex[0]);
			}
			else
			{
				simplex = std::vector<glm::vec3>{ simplex[0] };
				direction = -simplex[0];
			}
			break;

		case 3:
			{
				//	simplex = { C, B, A}
				glm::vec3 AB = simplex[1] - simplex[2];
				glm::vec3 AC = simplex[0] - simplex[2];
				glm::vec3 n = glm::cross(AB, AC);

				if (glm::dot(glm::cross(n, AC), -simplex[2]) > 0)
				{
					if (glm::dot(AC, -simplex[2]) > 0)
					{
						simplex = std::vector<glm::vec3>{ simplex[0], simplex[2] };
						direction = glm::cross(glm::cross(AC, -simplex[2]), AC);
					}
					else
					{
						if (glm::dot(AB, -simplex[2]) > 0)
						{
							simplex = std::vector<glm::vec3>{ simplex[1], simplex[2] };
							direction = glm::cross(glm::cross(AB, -simplex[2]), AB);
						}
						else
						{
							simplex = std::vector<glm::vec3>{ simplex[2] };
							direction = -simplex[2];
						}
					}
				}
				else
				{
					if (glm::dot(glm::cross(AB, n), -simplex[2]) > 0)
					{
						if (glm::dot(AB, -simplex[2]) > 0)
						{
							simplex = std::vector<glm::vec3>{ simplex[1], simplex[2] };
							direction = glm::cross(glm::cross(AB, -simplex[2]), AB);
						}
						else
						{
							simplex = std::vector<glm::vec3>{ simplex[2] };
							direction = -simplex[2];
						}
					}
					else
					{
						if (glm::dot(n, -simplex[2]) > 0)
							direction = n;
						else
							direction = -n;
					}
				}
			}
			break;

		case 4:
			{
				glm::vec3 n1 = glm::normalize(glm::cross(simplex[1] - simplex[0], simplex[2] - simplex[0]));
				if (glm::dot(n1, simplex[3] - simplex[0]) > 0) n1 *= -1.f;
				glm::vec3 n2 = glm::normalize(glm::cross(simplex[1] - simplex[0], simplex[3] - simplex[0]));
				if (glm::dot(n2, simplex[2] - simplex[0]) > 0) n2 *= -1.f;
				glm::vec3 n3 = glm::normalize(glm::cross(simplex[3] - simplex[2], simplex[1] - simplex[2]));
				if (glm::dot(n3, simplex[0] - simplex[2]) > 0) n3 *= -1.f;
				glm::vec3 n4 = glm::normalize(glm::cross(simplex[3] - simplex[2], simplex[0] - simplex[2]));
				if (glm::dot(n4, simplex[1] - simplex[2]) > 0) n4 *= -1.f;

				float d1 = glm::dot(n1, -simplex[0]);
				if (d1 < 0) d1 = std::numeric_limits<float>::max();
				float d2 = glm::dot(n2, -simplex[0]);
				if (d2 < 0) d2 = std::numeric_limits<float>::max();
				float d3 = glm::dot(n3, -simplex[2]);
				if (d3 < 0) d3 = std::numeric_limits<float>::max();
				float d4 = glm::dot(n4, -simplex[2]);
				if (d4 < 0) d4 = std::numeric_limits<float>::max();

				if (d1 < d2 && d1 < d3 && d1 < d4)
				{
					simplex = std::vector<glm::vec3>{ simplex[0], simplex[1], simplex[2] };
					direction = n1;
				}
				else if (d2 < d1 && d2 < d3 && d2 < d4)
				{
					simplex = std::vector<glm::vec3>{ simplex[0], simplex[1], simplex[3] };
					direction = n2;
				}
				else if (d3 < d1 && d3 < d2 && d3 < d4)
				{
					 simplex = std::vector<glm::vec3>{ simplex[1], simplex[2], simplex[3] };
					 direction = n3;
				}
				else
				{
					simplex = std::vector<glm::vec3>{ simplex[0], simplex[2], simplex[3] };
					direction = n4;
				}
			}
			break;

		default:
			std::cout << "GJK : error : simplex size not supported" << std::endl;
			break;
	}
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

		if (glm::dot(n1, -simplex[0]) > 0) return false;
		else if (glm::dot(n2, -simplex[0]) > 0) return false;
		else if (glm::dot(n3, -simplex[2]) > 0) return false;
		else if (glm::dot(n4, -simplex[2]) > 0) return false;
		else return true;
	}
	else 
	{
		std::cout << "GJK : error : simplex check not possible in normal case (dimension too high or too low)" << std::endl;
		return true;
	}
}
//