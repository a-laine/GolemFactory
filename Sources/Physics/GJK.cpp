#include "GJK.h"
#include "SpecificCollision/CollisionPoint.h"
#include "SpecificCollision/CollisionUtils.h"

//#include "SpecificIntersection/IntersectionPoint.h"

#include "Utiles/Debug.h"

#include <iostream>

#define MAX_ITERATION 50

//	Public functions
bool GJK::collide(const Shape& a, const Shape& b, std::vector<std::pair<glm::vec3, glm::vec3>>* shapePair)
{
	// map optionnal result
	std::vector<std::pair<glm::vec3, glm::vec3>> tmp;
	std::vector<std::pair<glm::vec3, glm::vec3>>& simplexPoints = shapePair ? *shapePair : tmp;

	// initialize loop
	glm::vec3 direction = glm::vec3(1, 0, 0);
	simplexPoints = { {a.GJKsupport(direction), b.GJKsupport(-direction)} };
	glm::vec3 S = simplexPoints[0].first - simplexPoints[0].second;
	std::vector<glm::vec3> simplex;
	simplex.reserve(4);
	simplex.push_back(S);
	direction = -direction;

	// iterate
	for(unsigned int i=0; i < MAX_ITERATION; i++)
	{
		glm::vec3 A = a.GJKsupport(direction);
		glm::vec3 B = b.GJKsupport(-direction);
		S = A - B;

		if (!isNewPoint(S, simplex))
			return false;

		simplex.push_back(S);
		simplexPoints.push_back({ A, B });

		if (glm::dot(S, direction) < 0)
			return false;
		if (containOrigin(simplex))
			return true;

		prepareSimplex(simplex, direction, simplexPoints);
	}

	//std::cout << "GJK : error : no solution found after maximum iteration (" << MAX_ITERATION << ")" << std::endl;
	return false;
}
Intersection::Contact GJK::intersect(const Shape& a, const Shape& b)
{
	std::vector<std::pair<glm::vec3, glm::vec3>> simplexPair;
	if (collide(a, b, &simplexPair))
	{
		std::vector<glm::vec3> simplex;
		simplex.reserve(4);
		for(unsigned int i=0; i<simplexPair.size(); i++)
			simplex.push_back(simplexPair[i].first - simplexPair[i].second);
		glm::vec3 direction(0.f);

		for (unsigned int i = 0; i < MAX_ITERATION; i++)
		{
			/*prepareSimplex(simplex, direction, simplexPair);
			glm::vec3 A = a.GJKsupport(direction);
			glm::vec3 B = b.GJKsupport(-direction);
			glm::vec3 S = A - B;

			if (!isNewPoint(S, simplex))
				break;
			simplex.push_back(S);
			simplexPair.push_back({ A, B });*/
		}
		/*
			start with collision tmp result
				1  f closest face of polyhedron
				2  extend polyhedron through f
				4  if not possible to extend : return f
				3  goto 1
		*/
	}
	else
	{

	}
	Intersection::Contact contact;

	// draw minkowski pairs
	Debug::color = Debug::orange;
	for (unsigned int i = 0; i < simplexPair.size(); i++)
		Debug::drawLine(simplexPair[i].first, simplexPair[i].second);

	// draw shifted origin
	glm::vec3 offset = glm::vec3(0,0,3);
	Debug::color = Debug::black;
	Debug::drawPoint(offset);

	// draw latest simplex
	Debug::color = Debug::magenta;
	switch(simplexPair.size())
	{
		case 1:
			Debug::drawPoint(simplexPair[0].first - simplexPair[0].second + offset);
			contact = Intersection::intersect_PointvsPoint(simplexPair[0].first, simplexPair[0].second);
			break;
		case 2:
			Debug::drawLine(simplexPair[0].first - simplexPair[0].second + offset, simplexPair[1].first - simplexPair[1].second + offset);
			Debug::color = Debug::red;
			Debug::drawLine(getSegmentClosestPoint(simplexPair[0].first - simplexPair[0].second + offset, simplexPair[1].first - simplexPair[1].second + offset, offset), offset);
			contact = Intersection::intersect_SegmentvsSegment(simplexPair[0].first, simplexPair[1].first, simplexPair[0].second, simplexPair[1].second);
			break;
		case 3:
			{
				glm::vec3 p1 = simplexPair[0].first - simplexPair[0].second + offset;
				glm::vec3 p2 = simplexPair[1].first - simplexPair[1].second + offset;
				glm::vec3 p3 = simplexPair[2].first - simplexPair[2].second + offset;
				glm::vec3 n1 = glm::cross(p2 - p1, p3 - p1);

				Debug::color = Debug::magenta;
				Debug::drawLine(p1, p2);
				Debug::drawLine(p1, p3);
				Debug::drawLine(p3, p2);

				Debug::color = Debug::blue;
				Debug::drawLine(0.3333f*(p1 + p2 + p3), 0.3333f*(p1 + p2 + p3) + 0.15f*glm::normalize(n1));

				Debug::color = Debug::red;
				Intersection::Contact c = Intersection::intersect_PointvsTriangle(offset, p1, p2, p3);
				Debug::drawLine(c.contactPointA, c.contactPointB);
			}
			break;
		default:
			{
				glm::vec3 p1 = simplexPair[0].first - simplexPair[0].second + offset;
				glm::vec3 p2 = simplexPair[1].first - simplexPair[1].second + offset;
				glm::vec3 p3 = simplexPair[2].first - simplexPair[2].second + offset;
				glm::vec3 p4 = simplexPair[3].first - simplexPair[3].second + offset;

				glm::vec3 n1 = glm::cross(p2 - p1, p3 - p1);
				if (glm::dot(n1, p4 - p1) > 0) n1 *= -1.f;
				glm::vec3 n2 = glm::cross(p2 - p1, p4 - p1);
				if (glm::dot(n2, p3 - p1) > 0) n2 *= -1.f;
				glm::vec3 n3 = glm::cross(p4 - p3, p2 - p3);
				if (glm::dot(n3, p1 - p3) > 0) n3 *= -1.f;
				glm::vec3 n4 = glm::cross(p4 - p3, p1 - p3);
				if (glm::dot(n4, p2 - p3) > 0) n4 *= -1.f;

				Debug::color = Debug::magenta;
				Debug::drawLine(p1, p2);
				Debug::drawLine(p1, p3);
				Debug::drawLine(p1, p4);
				Debug::drawLine(p2, p3);
				Debug::drawLine(p2, p4);
				Debug::drawLine(p3, p4);

				Debug::color = Debug::blue;
				Debug::drawLine(0.3333f*(p1 + p2 + p3), 0.3333f*(p1 + p2 + p3) + 0.15f*glm::normalize(n1));
				Debug::drawLine(0.3333f*(p1 + p2 + p4), 0.3333f*(p1 + p2 + p4) + 0.15f*glm::normalize(n2));
				Debug::drawLine(0.3333f*(p2 + p4 + p3), 0.3333f*(p2 + p4 + p3) + 0.15f*glm::normalize(n3));
				Debug::drawLine(0.3333f*(p1 + p4 + p3), 0.3333f*(p1 + p4 + p3) + 0.15f*glm::normalize(n4));

				Debug::color = Debug::red;
				Intersection::Contact c, tmp;
				float dmin = std::numeric_limits<float>::max();
				std::vector<int> faceIndex;

				tmp = Intersection::intersect_PointvsTriangle(offset, p1, p2, p3);
				if (float d = glm::length2(tmp.contactPointA - tmp.contactPointB) < dmin)
				{
					c = tmp;
					dmin = d;
					faceIndex = { 0,1,2 };
				}
				tmp = Intersection::intersect_PointvsTriangle(offset, p1, p2, p4);
				if (float d = glm::length2(tmp.contactPointA - tmp.contactPointB) < dmin)
				{
					c = tmp;
					dmin = d;
					faceIndex = { 0,1,3 };
				}
				tmp = Intersection::intersect_PointvsTriangle(offset, p4, p2, p3);
				if (float d = glm::length2(tmp.contactPointA - tmp.contactPointB) < dmin)
				{
					c = tmp;
					dmin = d;
					faceIndex = { 3,1,2 };
				}
				tmp = Intersection::intersect_PointvsTriangle(offset, p1, p4, p3);
				if (float d = glm::length2(tmp.contactPointA - tmp.contactPointB) < dmin)
				{
					c = tmp;
					dmin = d;
					faceIndex = { 0,3,2 };
				}
				Debug::drawLine(c.contactPointA, c.contactPointB);

				// get contact in minkowski polyhedron
				p1 = simplexPair[faceIndex[0]].first - simplexPair[faceIndex[0]].second;
				p2 = simplexPair[faceIndex[1]].first - simplexPair[faceIndex[1]].second;
				p3 = simplexPair[faceIndex[2]].first - simplexPair[faceIndex[2]].second;

				c = Intersection::intersect_PointvsTriangle(glm::vec3(0.f), p1, p2, p3);

				// compute barrycentric coordinates
				glm::vec2 b = getBarycentricCoordinates(p2 - p1, p3 - p1, c.contactPointB - p1);

				contact.contactPointA = simplexPair[faceIndex[0]].first + 
					b.x * (simplexPair[faceIndex[1]].first - simplexPair[faceIndex[0]].first) + 
					b.y * (simplexPair[faceIndex[2]].first - simplexPair[faceIndex[0]].first);
				contact.contactPointB = simplexPair[faceIndex[0]].second +
					b.x * (simplexPair[faceIndex[1]].second - simplexPair[faceIndex[0]].second) +
					b.y * (simplexPair[faceIndex[2]].second - simplexPair[faceIndex[0]].second);
			}
			break;
	}
	

	return contact;
}
//


//	Private functions
void GJK::prepareSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction, std::vector<std::pair<glm::vec3, glm::vec3>>& simplexPoints)
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
				simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[0] };
				direction = -simplex[0];
			}
			break;

		case 3:
			{
				//	simplex = { C, B, A}
				glm::vec3 AB = simplex[1] - simplex[2];
				glm::vec3 AC = simplex[0] - simplex[2];
				glm::vec3 n = glm::cross(AB, AC);
				//if (glm::dot(n, -simplex[2]) < 0.f) n *= -1.f;

				if (glm::dot(glm::cross(n, AC), -simplex[2]) > 0)
				{
					if (glm::dot(AC, -simplex[2]) > 0)
					{
						simplex = std::vector<glm::vec3>{ simplex[0], simplex[2] };
						simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[0], simplexPoints[2] };
						direction = glm::cross(glm::cross(AC, -simplex[2]), AC);
					}
					else
					{
						if (glm::dot(AB, -simplex[2]) > 0)
						{
							simplex = std::vector<glm::vec3>{ simplex[1], simplex[2] };
							simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[1], simplexPoints[2] };
							direction = glm::cross(glm::cross(AB, -simplex[2]), AB);
						}
						else
						{
							simplex = std::vector<glm::vec3>{ simplex[2] };
							simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[2] };
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
							simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[1], simplexPoints[2] };
							direction = glm::cross(glm::cross(AB, -simplex[2]), AB);
						}
						else
						{
							simplex = std::vector<glm::vec3>{ simplex[2] };
							simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[2] };
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
				// compute and orient normals
				glm::vec3 n1 = glm::normalize(glm::cross(simplex[1] - simplex[0], simplex[2] - simplex[0]));
				if (glm::dot(n1, simplex[3] - simplex[0]) > 0) n1 *= -1.f;
				glm::vec3 n2 = glm::normalize(glm::cross(simplex[1] - simplex[0], simplex[3] - simplex[0]));
				if (glm::dot(n2, simplex[2] - simplex[0]) > 0) n2 *= -1.f;
				glm::vec3 n3 = glm::normalize(glm::cross(simplex[3] - simplex[2], simplex[1] - simplex[2]));
				if (glm::dot(n3, simplex[0] - simplex[2]) > 0) n3 *= -1.f;
				glm::vec3 n4 = glm::normalize(glm::cross(simplex[3] - simplex[2], simplex[0] - simplex[2]));
				if (glm::dot(n4, simplex[1] - simplex[2]) > 0) n4 *= -1.f;

				// compute distance of faces regardless origin
				float d1 = glm::dot(n1, -simplex[0]);
				if (d1 < 0) d1 = std::numeric_limits<float>::max();
				float d2 = glm::dot(n2, -simplex[0]);
				if (d2 < 0) d2 = std::numeric_limits<float>::max();
				float d3 = glm::dot(n3, -simplex[2]);
				if (d3 < 0) d3 = std::numeric_limits<float>::max();
				float d4 = glm::dot(n4, -simplex[2]);
				if (d4 < 0) d4 = std::numeric_limits<float>::max();

				// face 1 is closest
				if (d1 < d2 && d1 < d3 && d1 < d4)
				{
					simplex = std::vector<glm::vec3>{ simplex[0], simplex[1], simplex[2] };
					simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[0], simplexPoints[1], simplexPoints[2] };
					direction = n1;
				}

				// face 2 closest
				else if (d2 < d1 && d2 < d3 && d2 < d4)
				{
					simplex = std::vector<glm::vec3>{ simplex[0], simplex[1], simplex[3] };
					simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[0], simplexPoints[1], simplexPoints[3] };
					direction = n2;
				}

				// face 3
				else if (d3 < d1 && d3 < d2 && d3 < d4)
				{
					 simplex = std::vector<glm::vec3>{ simplex[1], simplex[2], simplex[3] };
					 simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[1], simplexPoints[2], simplexPoints[3] };
					 direction = n3;
				}

				//face 4
				else
				{
					simplex = std::vector<glm::vec3>{ simplex[0], simplex[2], simplex[3] };
					simplexPoints = std::vector<std::pair<glm::vec3, glm::vec3>>{ simplexPoints[0], simplexPoints[2], simplexPoints[3] };
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
		// compute and orien simplex normals
		glm::vec3 n1 = glm::cross(simplex[1] - simplex[0], simplex[2] - simplex[0]);
		if (glm::dot(n1, simplex[3] - simplex[0]) > 0) n1 *= -1.f;
		glm::vec3 n2 = glm::cross(simplex[1] - simplex[0], simplex[3] - simplex[0]);
		if (glm::dot(n2, simplex[2] - simplex[0]) > 0) n2 *= -1.f;
		glm::vec3 n3 = glm::cross(simplex[3] - simplex[2], simplex[1] - simplex[2]);
		if (glm::dot(n3, simplex[0] - simplex[2]) > 0) n3 *= -1.f;
		glm::vec3 n4 = glm::cross(simplex[3] - simplex[2], simplex[0] - simplex[2]);
		if (glm::dot(n4, simplex[1] - simplex[2]) > 0) n4 *= -1.f;

		// test against origin
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
bool GJK::isNewPoint(const glm::vec3& point, std::vector<glm::vec3>& simplex)
{
	for (int i = 0; i < simplex.size(); i++)
		if (simplex[i] == point)
			return false;
	return true;
}
//