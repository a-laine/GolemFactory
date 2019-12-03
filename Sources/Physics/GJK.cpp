#include "GJK.h"
#include "SpecificCollision/CollisionPoint.h"
#include "SpecificCollision/CollisionUtils.h"

//#include "SpecificIntersection/IntersectionPoint.h"

#include "Utiles/Debug.h"

#include <iostream>

#define MAX_ITERATION 50
#define EPSILON 0.00001f

int GJK::max_iteration = 0;


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
	glm::vec3 offset = glm::vec3(0, 0, 3);

	GJKHull hull;
	std::vector<std::pair<glm::vec3, glm::vec3>> simplexPair;
	Intersection::Contact contact;
	if (collide(a, b, &simplexPair))
	{
		/*
			start with collision tmp result
				1  f closest face of polyhedron
				2  extend polyhedron through f
				4  if not possible to extend : return f
				3  goto 1
		*/

		hull.init(simplexPair);
		for (unsigned short i = 0; i < max_iteration; i++)
		{
			glm::vec3 direction = hull.getClosestFromOrigin()->n;
			bool stop = hull.add(a.GJKsupport(direction), b.GJKsupport(-direction));
			if (stop) break;
		}

		GJKHull::Face& f = *hull.getClosestFromOrigin();
		auto c = Intersection::intersect_PointvsTriangle(glm::vec3(0), f.a1 - f.b1, f.a2 - f.b2, f.a3 - f.b3);
		Debug::color = Debug::red;
		Debug::drawLine(c.contactPointA + offset, c.contactPointB + offset);

		// get contact in minkowski polyhedron
		glm::vec3 p1 = f.a1 - f.b1;
		glm::vec3 p2 = f.a2 - f.b2;
		glm::vec3 p3 = f.a3 - f.b3;
		c = Intersection::intersect_PointvsTriangle(glm::vec3(0.f), p1, p2, p3);
		glm::vec2 b = getBarycentricCoordinates(p2 - p1, p3 - p1, c.contactPointB - p1);

		contact.contactPointA = f.a1 + b.x * (f.a2 - f.a1) + b.y * (f.a3 - f.a1);
		contact.contactPointB = f.b1 + b.x * (f.b2 - f.b1) + b.y * (f.b3 - f.b1);
	}
	else
	{
		if (simplexPair.size() > 3) hull.init(simplexPair);

		/*glm::vec3 face[6];
		glm::vec3 direction;
		getIntersectStart(simplexPair, direction, face);*/
	}



	Debug::color = Debug::black;
	Debug::drawPoint(offset);
	hull.draw(offset);
	return contact;







	// draw minkowski pairs
	Debug::color = Debug::orange;
	for (unsigned int i = 0; i < simplexPair.size(); i++)
		Debug::drawLine(simplexPair[i].first, simplexPair[i].second);

	// draw shifted origin
	//glm::vec3 offset = glm::vec3(0,0,3);
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
				// prepare aliases
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

				// draw tetrahedron
				Debug::color = Debug::magenta;
				Debug::drawLine(p1, p2);
				Debug::drawLine(p1, p3);
				Debug::drawLine(p1, p4);
				Debug::drawLine(p2, p3);
				Debug::drawLine(p2, p4);
				Debug::drawLine(p3, p4);

				// draw normals
				Debug::color = Debug::blue;
				Debug::drawLine(0.3333f*(p1 + p2 + p3), 0.3333f*(p1 + p2 + p3) + 0.15f*glm::normalize(n1));
				Debug::drawLine(0.3333f*(p1 + p2 + p4), 0.3333f*(p1 + p2 + p4) + 0.15f*glm::normalize(n2));
				Debug::drawLine(0.3333f*(p2 + p4 + p3), 0.3333f*(p2 + p4 + p3) + 0.15f*glm::normalize(n3));
				Debug::drawLine(0.3333f*(p1 + p4 + p3), 0.3333f*(p1 + p4 + p3) + 0.15f*glm::normalize(n4));

				// draw origin to tetrahedron distance
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
void GJK::getIntersectStart(std::vector<std::pair<glm::vec3, glm::vec3>>& simplexPoints, glm::vec3& direction, glm::vec3* face)
{
	switch (simplexPoints.size())
	{
		case 1: case 2:
			std::cout << "GJK Intersection : Error on simplex start" << std::endl;
			break;

		case 3:
			{
				glm::vec3 AB = (simplexPoints[1].first - simplexPoints[1].second) - (simplexPoints[2].first - simplexPoints[2].second);
				glm::vec3 AC = (simplexPoints[0].first - simplexPoints[0].second) - (simplexPoints[2].first - simplexPoints[2].second);
				direction = glm::cross(AB, AC);
				if (glm::dot(direction, -(simplexPoints[2].first - simplexPoints[2].second)) < 0)
					direction *= -1.f;
			}
			break;

		case 4:
			{
				glm::vec3 p0 = simplexPoints[0].first - simplexPoints[0].second;
				glm::vec3 p1 = simplexPoints[1].first - simplexPoints[1].second;
				glm::vec3 p2 = simplexPoints[2].first - simplexPoints[2].second;
				glm::vec3 p3 = simplexPoints[3].first - simplexPoints[3].second;

				glm::vec3 n0 = glm::normalize(glm::cross(p0 - p2, p1 - p2)); if (glm::dot(n0, p3 - p2) > 0) n0 *= -1.f;
				glm::vec3 n1 = glm::normalize(glm::cross(p0 - p3, p1 - p3)); if (glm::dot(n1, p2 - p3) > 0) n1 *= -1.f;
				glm::vec3 n2 = glm::normalize(glm::cross(p0 - p2, p3 - p2)); if (glm::dot(n2, p1 - p2) > 0) n2 *= -1.f;
				glm::vec3 n3 = glm::normalize(glm::cross(p3 - p2, p1 - p2)); if (glm::dot(n3, p0 - p2) > 0) n3 *= -1.f;

				float d0 = glm::abs(glm::dot(n0, -p2));
				float d1 = glm::abs(glm::dot(n1, -p3));
				float d2 = glm::abs(glm::dot(n2, -p2));
				float d3 = glm::abs(glm::dot(n3, -p2));

				if (d0 < d1 && d0 < d2 && d0 < d3)
				{
					direction = n0;

					face[0] = simplexPoints[0].first;
					face[1] = simplexPoints[1].first;
					face[2] = simplexPoints[2].first;
					face[3] = simplexPoints[0].second;
					face[4] = simplexPoints[1].second;
					face[5] = simplexPoints[2].second;
				}
				else if (d1 < d0 && d1 < d2 && d1 < d3)
				{
					direction = n1;

					face[0] = simplexPoints[0].first;
					face[1] = simplexPoints[1].first;
					face[2] = simplexPoints[3].first;
					face[3] = simplexPoints[0].second;
					face[4] = simplexPoints[1].second;
					face[5] = simplexPoints[3].second;
				}
				else if (d2 < d0 && d2 < d1 && d2 < d3)
				{
					direction = n2;

					face[0] = simplexPoints[1].first;
					face[1] = simplexPoints[2].first;
					face[2] = simplexPoints[3].first;
					face[3] = simplexPoints[1].second;
					face[4] = simplexPoints[2].second;
					face[5] = simplexPoints[3].second;
				}
				else
				{
					direction = n3;

					face[0] = simplexPoints[0].first;
					face[1] = simplexPoints[2].first;
					face[2] = simplexPoints[3].first;
					face[3] = simplexPoints[0].second;
					face[4] = simplexPoints[2].second;
					face[5] = simplexPoints[3].second;
				}
			}
	}
}
//

//	Nested classes
void GJK::GJKHull::init(std::vector<std::pair<glm::vec3, glm::vec3>> simplex)
{
	points.clear();
	edges.clear();
	faces.clear();

	glm::vec3 p1 = simplex[0].first - simplex[0].second;
	glm::vec3 p2 = simplex[1].first - simplex[1].second;
	glm::vec3 p3 = simplex[2].first - simplex[2].second;
	glm::vec3 p4 = simplex[3].first - simplex[3].second;

	glm::vec3 n1 = glm::cross(p2 - p1, p3 - p1);
	if (glm::dot(n1, p4 - p1) > 0) n1 *= -1.f;
	glm::vec3 n2 = glm::cross(p2 - p1, p4 - p1);
	if (glm::dot(n2, p3 - p1) > 0) n2 *= -1.f;
	glm::vec3 n3 = glm::cross(p4 - p3, p2 - p3);
	if (glm::dot(n3, p1 - p3) > 0) n3 *= -1.f;
	glm::vec3 n4 = glm::cross(p4 - p3, p1 - p3);
	if (glm::dot(n4, p2 - p3) > 0) n4 *= -1.f;

	Face f1;
	f1.a1 = simplex[0].first; f1.b1 = simplex[0].second;
	f1.a2 = simplex[1].first; f1.b2 = simplex[1].second;
	f1.a3 = simplex[2].first; f1.b3 = simplex[2].second;
	f1.n = n1;
	
	Face f2;
	f2.a1 = simplex[0].first; f2.b1 = simplex[0].second;
	f2.a2 = simplex[1].first; f2.b2 = simplex[1].second;
	f2.a3 = simplex[3].first; f2.b3 = simplex[3].second;
	f2.n = n2;

	Face f3;
	f3.a1 = simplex[1].first; f3.b1 = simplex[1].second;
	f3.a2 = simplex[2].first; f3.b2 = simplex[2].second;
	f3.a3 = simplex[3].first; f3.b3 = simplex[3].second;
	f3.n = n3;

	Face f4;
	f4.a1 = simplex[0].first; f4.b1 = simplex[0].second;
	f4.a2 = simplex[2].first; f4.b2 = simplex[2].second;
	f4.a3 = simplex[3].first; f4.b3 = simplex[3].second;
	f4.n = n4;
	 
	Edge e1(p1, p2);
	Edge e2(p2, p3);
	Edge e3(p1, p3);
	Edge e4(p4, p1);
	Edge e5(p4, p2);
	Edge e6(p4, p3);

	faces.push_back(f1);
	faces.push_back(f2);
	faces.push_back(f3);
	faces.push_back(f4);

	edges.push_back(e1);
	edges.push_back(e2);
	edges.push_back(e3);
	edges.push_back(e4);
	edges.push_back(e5);
	edges.push_back(e6);

	points.insert(Vertex(p1));
	points.insert(Vertex(p2));
	points.insert(Vertex(p3));
	points.insert(Vertex(p4));

	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		for (auto it2 = edges.begin(); it2 != edges.end(); it2++)
		{
			if (isFaceEdge(*it, *it2))
			{
				if (it->e1 == nullptr) it->e1 = &(*it2);
				else if (it->e2 == nullptr) it->e2 = &(*it2);
				else if (it->e3 == nullptr) it->e3 = &(*it2);
				else std::cout << "GJKHull : Fatal error ! : a face (triangle) has more than 3 edges" << std::endl;

				if (it2->f1 == nullptr) it2->f1 = &(*it);
				else if (it2->f2 == nullptr) it2->f2 = &(*it);
				else std::cout << "GJKHull : Fatal error ! : an edge has more than 2 faces" << std::endl;
			}
			if (it->e3 != nullptr)
				break;
		}
		if (!it->e3)
			std::cout << "GJKHull : Fatal error ! : a face (triangle) doesn't have 3 edges" << std::endl;
	}

	/*for (auto it = faces.begin(); it != faces.end(); it++)
	{
		std::cout << "f " << (unsigned long)&(*it) << " : " << std::endl;
		std::cout << "   " << (unsigned long)(it->e1) << std::endl;
		std::cout << "   " << (unsigned long)(it->e2) << std::endl;
		std::cout << "   " << (unsigned long)(it->e3) << std::endl;
	}
	for (auto it = edges.begin(); it != edges.end(); it++)
	{
		std::cout << "e " << (unsigned long)&(*it) << " : " << std::endl;
		std::cout << "   " << (unsigned long)(it->f1) << std::endl;
		std::cout << "   " << (unsigned long)(it->f2) << std::endl;
	}
	std::cout << std::endl << std::endl;*/
}
bool GJK::GJKHull::add(const glm::vec3& a, const glm::vec3& b)
{
	//	test if point inside current hull or already existing
	glm::vec3 p = a - b;
	bool inside = true;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (glm::dot(it->n, p - (it->a1 - it->b1)) > EPSILON)
		{
			inside = false;
			break;
		}
	}
	if (inside) return true;
	if (!points.insert(Vertex(p)).second) return true;
	
	//	compute horizon
	for (auto it = edges.begin(); it != edges.end(); it++)
		it->horizonCheck = 2;
	for (auto it = faces.begin(); it != faces.end(); it++)
		it->onHull = true;
	std::list<Edge*> horizon = computeHorizon(p);

	//	compute cone faces
	for (auto it = horizon.begin(); it != horizon.end(); it++)
	{
		//  create face vertex aliases
		Face* otherFace = nullptr;
		if ((*it)->f1) otherFace = (*it)->f1;
		else if ((*it)->f2) otherFace = (*it)->f2;
		else
		{
			std::cout << "GJKHull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			return true;
		}

		glm::vec3 a1 = otherFace->a1;
		glm::vec3 b1 = otherFace->b1;
		glm::vec3 a2 = otherFace->a2;
		glm::vec3 b2 = otherFace->b2;

		//	create cone face and add it to hull
		faces.insert(faces.end(), Face(a1, a2, a, b1, b2, b, glm::normalize(glm::cross((*it)->p2 - (*it)->p1, p - (*it)->p1))));
		Face* face = &faces.back();
		if (checkFaceNormal(*face)) face->n *= -1.f;

		//  create new horizon edge
		if (!(*it)->f1) (*it)->f1 = face;
		else if (!(*it)->f2) (*it)->f2 = face;
		else
		{
			std::cout << "GJKHull : Fatal error in horizon cone : both faces of an horizon edge are already set" << std::endl;
			return true;
		}
		face->e1 = (*it);

		// create others edges
		Edge* e2 = existingEdge((*it)->p1, p);
		if (!e2)
		{
			edges.insert(edges.end(), Edge((*it)->p1, p));
			e2 = &edges.back();
		}
		if (!e2->f1) e2->f1 = face;
		else if (!e2->f2) e2->f2 = face;
		else
		{
			std::cout << "GJKHull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			return true;
		}
		face->e2 = e2;

		Edge* e3 = existingEdge((*it)->p2, p);
		if (!e3)
		{
			edges.insert(edges.end(), Edge((*it)->p2, p));
			e3 = &edges.back();
		}
		if (!e3->f1) e3->f1 = face;
		else if (!e3->f2) e3->f2 = face;
		else
		{
			std::cout << "GJKHull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			return true;
		}
		face->e3 = e3;
	}

	//	clear hull from dead entities
	for (auto it = edges.begin(); it != edges.end();)
	{
		if (it->horizonCheck == 0) it = edges.erase(it);
		else it++;
	}
	for (auto it = faces.begin(); it != faces.end();)
	{
		if (it->onHull) it++;
		else it = faces.erase(it);
	}
	return false;
}
GJK::GJKHull::Face* GJK::GJKHull::getClosestFromOrigin()
{
	float dmin = std::numeric_limits<float>::max();
	Face* face = nullptr;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		glm::vec3 p1 = it->a1 - it->b1;
		glm::vec3 p2 = it->a2 - it->b2;
		glm::vec3 p3 = it->a3 - it->b3;

		Intersection::Contact inter = Intersection::intersect_PointvsTriangle(glm::vec3(0), p1, p2, p3);
		float d = glm::length2(inter.contactPointB);

		if (d < dmin)
		{
			dmin = d;
			face = &(*it);
		}
	}
	return face;
}
void GJK::GJKHull::draw(const glm::vec3& offset)
{
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		glm::vec3 p1 = it->a1 - it->b1 + offset;
		glm::vec3 p2 = it->a2 - it->b2 + offset;
		glm::vec3 p3 = it->a3 - it->b3 + offset;
		glm::vec3 n1 = it->n;

		Debug::color = Debug::magenta;
		Debug::drawLine(p1, p2);
		Debug::drawLine(p1, p3);
		Debug::drawLine(p3, p2);

		Debug::color = Debug::blue;
		Debug::drawLine(0.3333f*(p1 + p2 + p3), 0.3333f*(p1 + p2 + p3) + 0.15f*glm::normalize(n1));
	}
}
bool GJK::GJKHull::isNewPair(const glm::vec3& a, const glm::vec3& b)
{
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (it->a1 == a || it->b1 == b) return false;
		else if (it->a2 == a || it->b2 == b) return false;
		else if (it->a3 == a || it->b3 == b) return false;
	}
	return true;
}
bool GJK::GJKHull::isNewPoint(const glm::vec3& p)
{
	return points.find(Vertex(p)) != points.cend();
}
bool GJK::GJKHull::isFaceEdge(const Face& f, const Edge& e)
{
	glm::vec3 p1 = f.a1 - f.b1;
	glm::vec3 p2 = f.a2 - f.b2;
	glm::vec3 p3 = f.a3 - f.b3;
	if (glm::length2(p1 - e.p1)<EPSILON && glm::length2(p2 - e.p2) < EPSILON) return true;			// just f.p3 is not in edge
	else if (glm::length2(p2 - e.p1) < EPSILON && glm::length2(p3 - e.p2) < EPSILON) return true;		// just f.p1 is not in edge
	else if (glm::length2(p1 - e.p1) < EPSILON && glm::length2(p3 - e.p2) < EPSILON) return true;		// just f.p2 is not in edge
	else if (glm::length2(p1 - e.p2) < EPSILON && glm::length2(p2 - e.p1) < EPSILON) return true;		// just f.p3 is not in edge
	else if (glm::length2(p2 - e.p2) < EPSILON && glm::length2(p3 - e.p1) < EPSILON) return true;		// just f.p1 is not in edge
	else if (glm::length2(p1 - e.p2) < EPSILON && glm::length2(p3 - e.p1) < EPSILON) return true;		// just f.p2 is not in edge
	else return false;
}
GJK::GJKHull::Edge* GJK::GJKHull::existingEdge(const glm::vec3& p1, const glm::vec3& p2)
{
	for (auto it = edges.begin(); it != edges.end(); it++)
	{
		if (it->p1 == p1 && it->p2 == p2)
			return &(*it);
		else if (it->p1 == p2 && it->p2 == p1)
			return &(*it);
	}
	return nullptr;
}
std::list<GJK::GJKHull::Edge*> GJK::GJKHull::computeHorizon(const glm::vec3& eye)
{
	std::list<Edge*> horizon;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (glm::dot(it->n, eye - (it->a1 - it->b1)) > 0)
		{
			it->onHull = false;

			it->e1->horizonCheck--;
			it->e2->horizonCheck--;
			it->e3->horizonCheck--;

			horizon.insert(horizon.end(), it->e1);
			horizon.insert(horizon.end(), it->e2);
			horizon.insert(horizon.end(), it->e3);

			if (it->e1->f1 == &(*it)) it->e1->f1 = nullptr;
			else if (it->e1->f2 == &(*it)) it->e1->f2 = nullptr;
			if (it->e2->f1 == &(*it)) it->e2->f1 = nullptr;
			else if (it->e2->f2 == &(*it)) it->e2->f2 = nullptr;
			if (it->e3->f1 == &(*it)) it->e3->f1 = nullptr;
			else if (it->e3->f2 == &(*it)) it->e3->f2 = nullptr;
		}
	}

	for (auto it = horizon.begin(); it != horizon.end(); )
	{
		if ((*it)->horizonCheck == 0) it = horizon.erase(it);
		else it++;
	}
	return horizon;
}
bool GJK::GJKHull::checkFaceNormal(const Face& f)
{
	glm::vec3 p = f.a1 - f.b1;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (glm::dot(f.n, it->a1 - it->b1 - p) > EPSILON) return true;
		else if (glm::dot(f.n, it->a2 - it->b2 - p) > EPSILON) return true;
		else if (glm::dot(f.n, it->a3 - it->b3 - p) > EPSILON) return true;
	}
	return false;
}
//