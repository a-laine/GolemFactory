#include "GJK.h"
#include "SpecificCollision/CollisionPoint.h"
#include "SpecificCollision/CollisionUtils.h"


#include "Utiles/Debug.h"

#include <iostream>

#define MAX_ITERATION 50
#define EPSILON 0.00001f

int GJK::max_iteration = 50;
bool GJK::verbose = false;
bool GJK::gizmos = false;

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

	if(verbose)
		std::cout << "GJK : error : no solution found after maximum iteration (" << MAX_ITERATION << ")" << std::endl;
	return false;
}
Intersection::Contact GJK::intersect(const Shape& a, const Shape& b)
{
	GJKHull hull;
	std::vector<std::pair<glm::vec3, glm::vec3>> simplexPair;
	Intersection::Contact contact;
	bool collision;
	if (collide(a, b, &simplexPair))
	{
		hull.initFromTetrahedron(simplexPair);
		collision = true;
	}
	else
	{
		/*if (simplexPair.size() > 3)
			hull.initFromTetrahedron(simplexPair);*/
		if (simplexPair.size() < 4)
		{
			glm::vec3 p1 = simplexPair[0].first - simplexPair[0].second;
			glm::vec3 p2 = simplexPair[1].first - simplexPair[1].second;

			if (simplexPair.size() == 2)
			{
				glm::vec3 p1 = simplexPair[0].first - simplexPair[0].second;
				glm::vec3 p2 = simplexPair[1].first - simplexPair[1].second;
				glm::vec3 direction = glm::cross(glm::cross(p2 - p1, -p1), p2 - p1);
				simplexPair.push_back(std::pair<glm::vec3, glm::vec3>({ a.GJKsupport(direction), b.GJKsupport(-direction) }));
			}

			glm::vec3 p3 = simplexPair[2].first - simplexPair[2].second;
			glm::vec3 direction = glm::cross(p2 - p1, p3 - p1);
			simplexPair.push_back(std::pair<glm::vec3, glm::vec3>({ a.GJKsupport(direction), b.GJKsupport(-direction) }));
			//hull.initFromTriangle(simplexPair);
		}

		hull.initFromTetrahedron(simplexPair);
		collision = false;
	}

	/*
		start with collision tmp result
			1  f closest face of polyhedron
			2  extend polyhedron through f
			4  if not possible to extend : return f
			3  goto 1
	*/
	unsigned short i;
	for (i = 0; i < max_iteration; i++)
	{
		glm::vec3 direction = hull.getDirection(collision);
		if (hull.add(a.GJKsupport(direction), b.GJKsupport(-direction)))
			break;
	}

	// get closest minkowski diff faces to origin
	GJKHull::Face& f = *hull.getClosestFace(collision);
	auto c = Intersection::intersect_PointvsTriangle(glm::vec3(0), f.p1, f.p2, f.p3);

	//debug
	if (gizmos)
	{
		glm::vec3 offset(0, 0, 3);
		Debug::color = Debug::black;
		Debug::drawPoint(offset);
		Debug::color = Debug::red;
		Debug::drawLine(c.contactPointA + offset, c.contactPointB + offset);
		hull.draw(offset);
		if (verbose)
			std::cout << i << " / " << max_iteration << std::endl;
	}


	// get contact in minkowski polyhedron
	c = Intersection::intersect_PointvsTriangle(glm::vec3(0.f), f.p1, f.p2, f.p3);
	glm::vec2 barr = getBarycentricCoordinates(f.p2 - f.p1, f.p3 - f.p1, c.contactPointB - f.p1);

	contact.contactPointA = f.a1 + barr.x * (f.a2 - f.a1) + barr.y * (f.a3 - f.a1);
	contact.contactPointB = f.b1 + barr.x * (f.b2 - f.b1) + barr.y * (f.b3 - f.b1);
	contact.normalA = glm::normalize(glm::cross(f.a2 - f.a1, f.a3 - f.a1));
	contact.normalB = glm::normalize(glm::cross(f.b2 - f.b1, f.b3 - f.b1));

	if (collision)
	{
		if (glm::dot(contact.normalA, contact.contactPointB - contact.contactPointA) > 0)
			contact.normalA *= -1.f;
		if (glm::dot(contact.normalB, contact.contactPointA - contact.contactPointB) > 0)
			contact.normalB *= -1.f;
	}
	else
	{
		if (glm::dot(contact.normalA, contact.contactPointB - contact.contactPointA) < 0)
			contact.normalA *= -1.f;
		if (glm::dot(contact.normalB, contact.contactPointA - contact.contactPointB) < 0)
			contact.normalB *= -1.f;
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
			if (verbose)
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
		if (verbose)
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

//	Nested classes
void GJK::GJKHull::initFromTetrahedron(std::vector<std::pair<glm::vec3, glm::vec3>> simplex)
{
	// clear previous values (needed if a shared GJKHull is used)
	points.clear();
	edges.clear();
	faces.clear();

	//	minkowski difference simplex points (from previous collision result)
	glm::vec3 p1 = simplex[0].first - simplex[0].second;
	glm::vec3 p2 = simplex[1].first - simplex[1].second;
	glm::vec3 p3 = simplex[2].first - simplex[2].second;
	glm::vec3 p4 = simplex[3].first - simplex[3].second;

	//	faces normals
	glm::vec3 n1 = glm::cross(p2 - p1, p3 - p1);
	if (glm::dot(n1, p4 - p1) > 0) n1 *= -1.f;
	glm::vec3 n2 = glm::cross(p2 - p1, p4 - p1);
	if (glm::dot(n2, p3 - p1) > 0) n2 *= -1.f;
	glm::vec3 n3 = glm::cross(p4 - p3, p2 - p3);
	if (glm::dot(n3, p1 - p3) > 0) n3 *= -1.f;
	glm::vec3 n4 = glm::cross(p4 - p3, p1 - p3);
	if (glm::dot(n4, p2 - p3) > 0) n4 *= -1.f;

	Face f1(simplex[0].first, simplex[1].first, simplex[2].first, simplex[0].second, simplex[1].second, simplex[2].second, n1); // p1, p2, p3
	Face f2(simplex[0].first, simplex[1].first, simplex[3].first, simplex[0].second, simplex[1].second, simplex[3].second, n2); // p1, p2, p4
	Face f3(simplex[1].first, simplex[2].first, simplex[3].first, simplex[1].second, simplex[2].second, simplex[3].second, n3); // p2, p3, p4
	Face f4(simplex[0].first, simplex[2].first, simplex[3].first, simplex[0].second, simplex[2].second, simplex[3].second, n4); // p1, p3, p4

	Edge e1(simplex[0].first, simplex[1].first, simplex[0].second, simplex[1].second);
	Edge e2(simplex[1].first, simplex[2].first, simplex[1].second, simplex[2].second);
	Edge e3(simplex[0].first, simplex[2].first, simplex[0].second, simplex[2].second);
	Edge e4(simplex[3].first, simplex[0].first, simplex[3].second, simplex[0].second);
	Edge e5(simplex[3].first, simplex[1].first, simplex[3].second, simplex[1].second);
	Edge e6(simplex[3].first, simplex[2].first, simplex[3].second, simplex[2].second);

	Face* f1ptr = &(*faces.insert(faces.end(), f1));
	Face* f2ptr = &(*faces.insert(faces.end(), f2));
	Face* f3ptr = &(*faces.insert(faces.end(), f3));
	Face* f4ptr = &(*faces.insert(faces.end(), f4));

	Edge* e1ptr = &(*edges.insert(edges.end(), e1));
	Edge* e2ptr = &(*edges.insert(edges.end(), e2));
	Edge* e3ptr = &(*edges.insert(edges.end(), e3));
	Edge* e4ptr = &(*edges.insert(edges.end(), e4));
	Edge* e5ptr = &(*edges.insert(edges.end(), e5));
	Edge* e6ptr = &(*edges.insert(edges.end(), e6));

	associate(f1ptr, e1ptr); associate(f1ptr, e2ptr); associate(f1ptr, e3ptr);
	associate(f2ptr, e1ptr); associate(f2ptr, e4ptr); associate(f2ptr, e5ptr);
	associate(f3ptr, e2ptr); associate(f3ptr, e5ptr); associate(f3ptr, e6ptr);
	associate(f4ptr, e3ptr); associate(f4ptr, e4ptr); associate(f4ptr, e6ptr);

	points.insert(Vertex(p1));
	points.insert(Vertex(p2));
	points.insert(Vertex(p3));
	points.insert(Vertex(p4));

	/*
	//	print
	for (auto it = faces.begin(); it != faces.end(); it++)
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
	std::cout << std::endl << std::endl;
	*/
}
void GJK::GJKHull::initFromTriangle(std::vector<std::pair<glm::vec3, glm::vec3>> simplex)
{
	// clear previous values (needed if a shared GJKHull is used)
	points.clear();
	edges.clear();
	faces.clear();

	// triangle verticies
	glm::vec3 p1 = simplex[0].first - simplex[0].second;
	glm::vec3 p2 = simplex[1].first - simplex[1].second;
	glm::vec3 p3 = simplex[2].first - simplex[2].second;
	glm::vec3 n1 = glm::cross(p2 - p1, p3 - p1);
	if (glm::dot(n1, -p1) < 0) n1 *= -1.f;

	//	faces & edges
	Face f1(simplex[0].first, simplex[1].first, simplex[2].first, simplex[0].second, simplex[1].second, simplex[2].second, n1);
	Edge e1(simplex[0].first, simplex[1].first, simplex[0].second, simplex[1].second);
	Edge e2(simplex[1].first, simplex[2].first, simplex[1].second, simplex[2].second);
	Edge e3(simplex[0].first, simplex[2].first, simplex[0].second, simplex[2].second);

	//	push & associate
	Face* f1ptr = &(*faces.insert(faces.end(), f1));
	Edge* e1ptr = &(*edges.insert(edges.end(), e1));
	Edge* e2ptr = &(*edges.insert(edges.end(), e2));
	Edge* e3ptr = &(*edges.insert(edges.end(), e3));
	associate(f1ptr, e1ptr); associate(f1ptr, e2ptr); associate(f1ptr, e3ptr);

	points.insert(Vertex(p1));
	points.insert(Vertex(p2));
	points.insert(Vertex(p3));


	/*
	//	print
	for (auto it = faces.begin(); it != faces.end(); it++)
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
	std::cout << std::endl << std::endl;
	*/
}
bool GJK::GJKHull::add(const glm::vec3& a, const glm::vec3& b)
{
	// special case : tetrahedron not yet initialized
	/*if (faces.size() == 1)
	{
		Face f = faces.front();
		std::vector<std::pair<glm::vec3, glm::vec3>> simplex;
		simplex.push_back({ f.a1, f.b1 });
		simplex.push_back({ f.a2, f.b2 });
		simplex.push_back({ f.a3, f.b3 });
		simplex.push_back({ a, b });
		initFromTetrahedron(simplex);
		return false;
	}*/

	//	test if point inside current hull or already existing
	glm::vec3 p = a - b;
	bool inside = true;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (glm::dot(it->n, p - it->p1) > EPSILON)
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
		//  create edges aliases
		glm::vec3 a1 = (*it)->a1;
		glm::vec3 b1 = (*it)->b1;
		glm::vec3 a2 = (*it)->a2;
		glm::vec3 b2 = (*it)->b2;

		//	create cone face and add it to hull
		faces.insert(faces.end(), Face(a1, a2, a, b1, b2, b, glm::normalize(glm::cross((*it)->p2 - (*it)->p1, p - (*it)->p1))));
		Face* face = &faces.back();
		if (checkFaceNormal(*face)) face->n *= -1.f;

		//  create new horizon edge
		if (!(*it)->f1) (*it)->f1 = face;
		else if (!(*it)->f2) (*it)->f2 = face;
		else
		{
			if (verbose)
				std::cout << "GJKHull : Fatal error in horizon cone : both faces of an horizon edge are already set" << std::endl;
			return true;
		}
		face->e1 = (*it);

		// create others edges
		Edge* e2 = existingEdge((*it)->p1, p);
		if (!e2)
		{
			edges.insert(edges.end(), Edge((*it)->a1, a, (*it)->b1, b));
			e2 = &edges.back();
		}
		if (!e2->f1) e2->f1 = face;
		else if (!e2->f2) e2->f2 = face;
		else
		{
			if (verbose)
				std::cout << "GJKHull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			return true;
		}
		face->e2 = e2;

		Edge* e3 = existingEdge((*it)->p2, p);
		if (!e3)
		{
			edges.insert(edges.end(), Edge((*it)->a2, a, (*it)->b2, b));
			e3 = &edges.back();
		}
		if (!e3->f1) e3->f1 = face;
		else if (!e3->f2) e3->f2 = face;
		else
		{
			if (verbose)
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
glm::vec3 GJK::GJKHull::getDirection(bool collision)
{
	float dmin = std::numeric_limits<float>::max();
	glm::vec3 direction;
	if (collision)
	{
		for (auto it = faces.begin(); it != faces.end(); it++)
		{
			float d = glm::abs(glm::dot(it->n, -it->p1));
			if (d < dmin)
			{
				dmin = d;
				direction = it->n;
			}
		}
	}
	else
	{
		for (auto it = faces.begin(); it != faces.end(); it++)
		{
			glm::vec3* e1 = nullptr;
			glm::vec3 *e2 = nullptr;
			Intersection::Contact inter = Intersection::intersect_PointvsTriangle(glm::vec3(0), it->p1, it->p2, it->p3, e1, e2);
			float d = glm::length2(inter.contactPointB);
			
			if (d < dmin)
			{
				dmin = d;

				if (!e1) // closest is a point in triangle
					direction = it->n;
				else if (!e2) // closest is a corner
					direction = -(*e1);
				else // closest is an edge
					direction = glm::cross(glm::cross((*e2) - (*e1), -(*e1)), (*e2) - (*e1));
			}
		}
	}
	return direction;
}
GJK::GJKHull::Face* GJK::GJKHull::getClosestFace(bool collision)
{
	float dmin = std::numeric_limits<float>::max();
	Face* face = nullptr;
	float d;

	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (collision)
		{
			d = glm::abs(glm::dot(it->n, -it->p1));
		}
		else
		{
			Intersection::Contact inter = Intersection::intersect_PointvsTriangle(glm::vec3(0), it->p1, it->p2, it->p3);
			d = glm::length2(inter.contactPointB);
		}

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
		glm::vec3 p1 = it->p1 + offset;
		glm::vec3 p2 = it->p2 + offset;
		glm::vec3 p3 = it->p3 + offset;
		//glm::vec3 n1 = it->n;

		Debug::color = Debug::magenta;
		Debug::drawLine(p1, p2);
		Debug::drawLine(p1, p3);
		Debug::drawLine(p3, p2);

		Debug::color = Debug::blue;
		Debug::drawLine(0.3333f*(p1 + p2 + p3), 0.3333f*(p1 + p2 + p3) + 0.15f*it->n);
	}
}
void GJK::GJKHull::associate(Face* f, Edge* e)
{
	if (f->e1 == nullptr) f->e1 = e;
	else if (f->e2 == nullptr) f->e2 = e;
	else if (f->e3 == nullptr) f->e3 = e;
	else if (verbose)std::cout << "GJKHull : Fatal error ! : a face (triangle) has more than 3 edges" << std::endl;

	if (e->f1 == nullptr) e->f1 = f;
	else if (e->f2 == nullptr) e->f2 = f;
	else if (verbose)std::cout << "GJKHull : Fatal error ! : an edge has more than 2 faces" << std::endl;
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
		if (glm::dot(it->n, eye - it->p1) >= 0)
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
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (glm::dot(f.n, it->p1 - f.p1) > EPSILON) return true;
		else if (glm::dot(f.n, it->p2 - f.p1) > EPSILON) return true;
		else if (glm::dot(f.n, it->p3 - f.p1) > EPSILON) return true;
	}
	return false;
}
//