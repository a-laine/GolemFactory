#include "GJK.h"
#include "Collision.h"
#include "SpecificCollision/CollisionUtils.h"

#include <Utiles/Debug.h>
//#include <glm/gtx/simd_vec4.hpp>

#include <iostream>

#define MAX_ITERATION 50
//#define EPSILON 0.00001f

//int GJK::max_iteration = 50;
bool GJK::verbose = false;
bool GJK::gizmos = true;


//	Public functions
bool GJK::collide(const Shape& a, const Shape& b, CollisionReport* report)
{
	// initialize GJK search
	vec4f direction = vec4f(1, 0, 0, 0);
	std::vector<MinkowskiPoint> simplex;
	simplex.reserve(4);

	// iterate
	for (unsigned int i = 0; i < MAX_ITERATION; i++)
	{
		MinkowskiPoint S = MinkowskiPoint(a.support(direction), b.support(-direction));
		if (!isNewPoint(simplex, S))
			return false;

		simplex.push_back(S);

		if (vec4f::dot(S.p, direction) < 0)
			return false;

		if (containOrigin(simplex, report == nullptr))
		{
			if (report)
			{
				report->collision = true;
				report->shape1 = (Shape*)&a;
				report->shape2 = (Shape*)&b;

				if (simplex.size() >= 4)
				{
					expandSimplex(a, b, report, simplex);
				}
				else
				{
					std::cout << "WTF" << std::endl;// error
				}
			}
			return true;
		}

		// simplex simplify and direction compute
		switch (simplex.size())
		{
			case 1:
				direction = -simplex[0].p;
				break;

			case 2:
				{
					if (vec4f::dot(simplex[1].p - simplex[0].p, -simplex[0].p) > 0)
						direction = vec4f::cross(vec4f::cross(simplex[1].p - simplex[0].p, -simplex[0].p), simplex[1].p - simplex[0].p);
					else
					{
						direction = -simplex[0].p;
						simplex.pop_back();
						//simplex = std::vector<MinkowskiPoint>{ simplex[0] };
					}
				}
				break;

			case 3:
				{
					//	simplex = { C, B, A}
					vec4f AB = simplex[1].p - simplex[2].p;
					vec4f AC = simplex[0].p - simplex[2].p;
					vec4f n = vec4f::cross(AB, AC);

					if (vec4f::dot(vec4f::cross(n, AC), -simplex[2].p) > 0)
					{
						if (vec4f::dot(AC, -simplex[2].p) > 0)
						{
							direction = vec4f::cross(vec4f::cross(AC, -simplex[2].p), AC);
							simplex[1] = simplex[2];
							simplex.pop_back();
							//simplex = std::vector<MinkowskiPoint>{ simplex[0], simplex[2] };
						}
						else
						{
							if (vec4f::dot(AB, -simplex[2].p) > 0)
							{
								direction = vec4f::cross(vec4f::cross(AB, -simplex[2].p), AB);
								simplex[0] = simplex[1];
								simplex[1] = simplex[2];
								simplex.pop_back();
								//simplex = std::vector<MinkowskiPoint>{ simplex[1], simplex[2] };
							}
							else
							{
								MinkowskiPoint p = simplex[2];
								simplex.clear();
								simplex.push_back(p);
								direction = -simplex[2].p;
								//simplex = std::vector<MinkowskiPoint>{ simplex[2] };
							}
						}
					}
					else
					{
						if (vec4f::dot(vec4f::cross(AB, n), -simplex[2].p) > 0)
						{
							if (vec4f::dot(AB, -simplex[2].p) > 0)
							{
								direction = vec4f::cross(vec4f::cross(AB, -simplex[2].p), AB);
								simplex[0] = simplex[1];
								simplex[1] = simplex[2];
								simplex.pop_back();
								//simplex = std::vector<MinkowskiPoint>{ simplex[1], simplex[2] };
							}
							else
							{
								MinkowskiPoint p = simplex[2];
								simplex.clear();
								simplex.push_back(p);
								direction = -p.p;
								//simplex = std::vector<MinkowskiPoint>{ simplex[2] };
							}
						}
						else
						{
							if (vec4f::dot(n, -simplex[2].p) > 0)
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
					vec4f n1 = vec4f::cross(simplex[1].p - simplex[0].p, simplex[2].p - simplex[0].p).getNormal();
					if (vec4f::dot(n1, simplex[3].p - simplex[0].p) > 0) n1 *= -1.f;
					vec4f n2 = vec4f::cross(simplex[1].p - simplex[0].p, simplex[3].p - simplex[0].p).getNormal();
					if (vec4f::dot(n2, simplex[2].p - simplex[0].p) > 0) n2 *= -1.f;
					vec4f n3 = vec4f::cross(simplex[3].p - simplex[2].p, simplex[1].p - simplex[2].p).getNormal();
					if (vec4f::dot(n3, simplex[0].p - simplex[2].p) > 0) n3 *= -1.f;
					vec4f n4 = vec4f::cross(simplex[3].p - simplex[2].p, simplex[0].p - simplex[2].p).getNormal();
					if (vec4f::dot(n4, simplex[1].p - simplex[2].p) > 0) n4 *= -1.f;

					// compute distance of faces regardless origin
					float d1 = vec4f::dot(n1, -simplex[0].p);
					if (d1 < 0) d1 = std::numeric_limits<float>::max();
					float d2 = vec4f::dot(n2, -simplex[0].p);
					if (d2 < 0) d2 = std::numeric_limits<float>::max();
					float d3 = vec4f::dot(n3, -simplex[2].p);
					if (d3 < 0) d3 = std::numeric_limits<float>::max();
					float d4 = vec4f::dot(n4, -simplex[2].p);
					if (d4 < 0) d4 = std::numeric_limits<float>::max();

					// face 1 is closest
					if (d1 < d2 && d1 < d3 && d1 < d4)
					{
						//simplex = std::vector<MinkowskiPoint>{ simplex[0], simplex[1], simplex[2] };
						simplex.pop_back();
						direction = n1;
					}

					// face 2 closest
					else if (d2 < d1 && d2 < d3 && d2 < d4)
					{
						//simplex = std::vector<MinkowskiPoint>{ simplex[0], simplex[1], simplex[3] };
						simplex[2] = simplex[3];
						simplex.pop_back();
						direction = n2;
					}

					// face 3
					else if (d3 < d1 && d3 < d2 && d3 < d4)
					{
						simplex[0] = simplex[1];
						simplex[1] = simplex[2];
						simplex[2] = simplex[3];
						simplex.pop_back();
						//simplex = std::vector<MinkowskiPoint>{ simplex[1], simplex[2], simplex[3] };
						direction = n3;
					}

					//face 4
					else
					{
						simplex[1] = simplex[2];
						simplex[2] = simplex[3];
						simplex.pop_back();
						//simplex = std::vector<MinkowskiPoint>{ simplex[0], simplex[2], simplex[3] };
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

	if (verbose)
		std::cout << "GJK : error : no solution found after maximum iteration (" << MAX_ITERATION << ")" << std::endl;
	return false;
}
void GJK::expandSimplex(const Shape& a, const Shape& b, CollisionReport* report, const std::vector<MinkowskiPoint>& simplex)
{
	GJKHull hull;
	hull.initFromTetrahedron(simplex);
	if (hull.faces.size() < 4)
	{
		// fatal error
		report->collision = false;
		return;
	}

	Face* closestFace;

	unsigned int i = 0;
	for (; i < MAX_ITERATION; i++)
	{
		closestFace = hull.getClosestFaceToOrigin();
		if (hull.add(MinkowskiPoint(a.support(closestFace->n), b.support(-closestFace->n))))
			break;
	}

	float depth = vec4f::dot(closestFace->n, closestFace->p1.p);
	vec4f proj = depth * closestFace->n;
	report->depths.push_back(std::abs(depth));

	vec2f barr = CollisionUtils::getBarycentricCoordinates(closestFace->p2.p - closestFace->p1.p, closestFace->p3.p - closestFace->p1.p, proj - closestFace->p1.p, true);

	vec4f at1 = closestFace->p1.a;
	vec4f at2 = closestFace->p2.a;
	vec4f at3 = closestFace->p3.a;

	vec4f u1 = at2 - at1;	float l1 = u1.getNorm();
	vec4f u2 = at3 - at1;	float l2 = u2.getNorm();
	vec4f u3 = at2 - at3;	float l3 = u3.getNorm();

	vec4f pa = at1 + barr.x * u1 + barr.y * u2;
	vec4f pb = closestFace->p1.b + barr.x * (closestFace->p2.b - closestFace->p1.b) + barr.y * (closestFace->p3.b - closestFace->p1.b);
	report->points.push_back(pa);

	if (l1 > COLLISION_EPSILON && l2 > COLLISION_EPSILON && l3 > COLLISION_EPSILON)
		report->normal = vec4f::cross(u1, u2).getNormal();
	else
	{
		vec4f edge;
		if (l1 > l2 && l1 > l3)
			edge = u1;
		else if (l2 > l1 && l2 > l3)
			edge = u2;
		else edge = u3;

		if (edge.getNorm2() > COLLISION_EPSILON)
		{
			edge.normalize();
			report->normal = (proj - vec4f::dot(proj, edge) * edge).getNormal();
		}
		else
			report->normal = proj.getNormal();
	}

	if (vec4f::dot(report->normal, proj) < 0.f)
		report->normal *= -1.f;

	if (report->computeManifoldContacts)
		computeManifoldContacts(a, b, report);
}
void GJK::computeManifoldContacts(const Shape& a, const Shape& b, CollisionReport* report)
{
	// the two feature faces
	a.getFacingFace(report->normal, report->shape1face);
	b.getFacingFace(-report->normal, report->shape2face);
	if (report->shape1face.size() < 3 || report->shape2face.size() < 3)
		return;

	if (gizmos)
	{
		const float pointRadius = 0.01f;
		Debug::color = Debug::green;
		for (unsigned int i = 0; i < report->shape1face.size(); i++)
			Debug::drawSphere(report->shape1face[i], pointRadius);

		Debug::color = Debug::blue;
		for (unsigned int i = 0; i < report->shape2face.size(); i++)
			Debug::drawSphere(report->shape2face[i], pointRadius);
	}

	vec4f originalPoint = report->points.back();
	float originalDepth = report->depths.back();
	report->points.clear();
	report->depths.clear();

	// compute faces normals
	vec4f n1 = vec4f::cross(report->shape1face[1]- report->shape1face[0], report->shape1face[2] - report->shape1face[0]).getNormal();
	if (vec4f::dot(n1, report->normal) < 0.f)
		n1 *= -1.f;
	vec4f n2 = vec4f::cross(report->shape2face[1] - report->shape2face[0], report->shape2face[2] - report->shape2face[0]).getNormal();
	if (vec4f::dot(n2, report->normal) < 0.f)
		n2 *= -1.f;
	float dot = vec4f::dot(n1, n2);

	// project face1 on face2 -> if corner is inside hull, it's a contact point
	std::vector<vec4f> projection;
	for (unsigned int i = 0; i < report->shape1face.size(); i++)
	{
		float depth = vec4f::dot(report->normal, report->shape1face[i] - report->shape2face[0]);
		vec4f p = report->shape1face[i] - depth * report->normal;
		projection.push_back(p);

		if (depth > 0.f && inside(report->shape2face, p))
		{
			report->points.push_back(p);
			report->depths.push_back(depth);
		}
	}

	// test face2 on projected hull -> if it's inside it's a contact point
	for (unsigned int i = 0; i < report->shape2face.size(); i++)
	{
		if (inside(projection, report->shape2face[i]))
		{
			float depth = vec4f::dot(report->shape1face[0] - report->shape2face[i], n1) / dot;
			report->points.push_back(report->shape2face[i] + depth * n2);
			report->depths.push_back(depth);
		}
	}

	// edge vs edge
	vec4f intersection;
	for (unsigned int i = 0; i < report->shape1face.size(); i++)
	{
		vec4f s1a = report->shape1face[i];
		vec4f s1b = report->shape1face[(i + 1) % report->shape1face.size()];
		for (unsigned int j = 0; j < report->shape2face.size(); j++)
		{
			vec4f s2a = report->shape2face[j];
			vec4f s2b = report->shape2face[(j + 1) % report->shape2face.size()];

			if (collide(s1a, s1b, s2a, s2b, intersection))
			{
				float depth = vec4f::dot(intersection - report->shape2face[0], n2);
				report->points.push_back(intersection);
				report->depths.push_back(depth);
			}
		}
	}

	// end
	if (report->points.empty())
	{
		report->points.push_back(originalPoint);
		report->depths.push_back(originalDepth);
	}
}
//


//	Private functions
bool GJK::isNewPoint(const std::vector<MinkowskiPoint>& simplex, const MinkowskiPoint& p)
{
	for (const MinkowskiPoint& point : simplex)
		if (point == p)
			return false;
	return true;
}
bool GJK::containOrigin(const std::vector<MinkowskiPoint>& simplex, const bool& earlyExit)
{
	if (simplex.size() < 3)
		return earlyExit && simplex[0].p == vec4f(0);
	else if (simplex.size() == 2)
		return earlyExit && Collision::collide_PointvsSegment(vec4f(0), simplex[0].p, simplex[1].p);
	else if (simplex.size() == 3)
		return earlyExit && Collision::collide_PointvsTriangle(vec4f(0), simplex[0].p, simplex[1].p, simplex[2].p);
	else if (simplex.size() == 4)
	{
		// compute and orien simplex normals
		vec4f n1 = vec4f::cross(simplex[1].p - simplex[0].p, simplex[2].p - simplex[0].p);
		if (vec4f::dot(n1, simplex[3].p - simplex[0].p) > 0) n1 *= -1.f;
		vec4f n2 = vec4f::cross(simplex[1].p - simplex[0].p, simplex[3].p - simplex[0].p);
		if (vec4f::dot(n2, simplex[2].p - simplex[0].p) > 0) n2 *= -1.f;
		vec4f n3 = vec4f::cross(simplex[3].p - simplex[2].p, simplex[1].p - simplex[2].p);
		if (vec4f::dot(n3, simplex[0].p - simplex[2].p) > 0) n3 *= -1.f;
		vec4f n4 = vec4f::cross(simplex[3].p - simplex[2].p, simplex[0].p - simplex[2].p);
		if (vec4f::dot(n4, simplex[1].p - simplex[2].p) > 0) n4 *= -1.f;

		// test against origin
		if (vec4f::dot(n1, -simplex[0].p) > 0) return false;
		else if (vec4f::dot(n2, -simplex[0].p) > 0) return false;
		else if (vec4f::dot(n3, -simplex[2].p) > 0) return false;
		else if (vec4f::dot(n4, -simplex[2].p) > 0) return false;
		else return true;
	}
	else
	{
		if (verbose)
			std::cout << "GJK::containOrigin : simplex check not possible in normal case (dimension too high or too low)" << std::endl;
		return true;
	}
}
bool GJK::collide(const vec4f& s1a, const vec4f& s1b, const vec4f& s2a, const vec4f& s2b, vec4f& intersection)
{
	vec4f u = s1b - s1a;
	vec4f v = s2b - s2a;
	vec4f w = s1a - s2a;

	float a = u.getNorm2();
	float b = vec4f::dot(u, v);
	float c = v.getNorm2();
	float d = vec4f::dot(u, w);
	float e = vec4f::dot(v, w);

	float D = a * c - b * b;
	if (D < COLLISION_EPSILON * COLLISION_EPSILON)
		return false;

	float t1 = (b * e - c * d) / D;
	float t2 = (a * e - b * d) / D;

	if (t1 > 1.f || t2 > 1.f || t1 < 0.f || t2 < 0.f)
		return false;

	intersection = s1a + t1 * u;
	return true;
}
bool GJK::inside(std::vector<vec4f>& hull, const vec4f& point)
{
	vec4f sign;
	for (unsigned int i = 0; i < hull.size(); i++)
	{
		vec4f u = hull[(i + 1) % hull.size()] - hull[i];
		vec4f v = point - hull[i];
		vec4f n = vec4f::cross(u, v);

		if (i == 0)
			sign = n;
		else if (vec4f::dot(n, sign) <= 0.f)
			return false;
	}
	return true;
}
//

//	Nested classes
GJK::MinkowskiPoint::MinkowskiPoint(const vec4f& _a, const vec4f& _b) : 
	a(_a), b(_b), p(_a - _b) 
{

}
bool GJK::MinkowskiPoint::operator==(const MinkowskiPoint& _other) const
{
	if (a != _other.a || b != _other.b)
		return false;
	else
		return true;
}
bool GJK::MinkowskiPoint::operator< (const MinkowskiPoint& _other) const
{
	if (p.x != _other.p.x)
		return p.x < _other.p.x;
	else if (p.y != _other.p.y)
		return p.y < _other.p.y;
	else
		return p.z < _other.p.z;
}



GJK::Face::Face(const MinkowskiPoint& _p1, const MinkowskiPoint& _p2, const MinkowskiPoint& _p3, const vec4f& _n) :
	p1(_p1), p2(_p2), p3(_p3), n(_n),
	onHull(true), e1(nullptr), e2(nullptr), e3(nullptr)
{}
GJK::Edge::Edge(const MinkowskiPoint& _p1, const MinkowskiPoint& _p2) :
	p1(_p1), p2(_p2), horizonCheck(2), f1(nullptr), f2(nullptr)
{}






void GJK::GJKHull::initFromTetrahedron(const std::vector<MinkowskiPoint>& simplex)
{
	// clear previous values (needed if a shared GJKHull is used)
	points.clear();
	edges.clear();
	faces.clear();

	points.insert(simplex[0]);
	points.insert(simplex[1]);
	points.insert(simplex[2]);
	points.insert(simplex[3]);

	//	faces normals
	vec4f n1 = vec4f::cross(simplex[1].p - simplex[0].p, simplex[2].p - simplex[0].p).getNormal();
	if (vec4f::dot(n1, simplex[3].p - simplex[0].p) > 0)
		n1 *= -1.f;

	vec4f n2 = vec4f::cross(simplex[1].p - simplex[0].p, simplex[3].p - simplex[0].p).getNormal();
	if (vec4f::dot(n2, simplex[2].p - simplex[0].p) > 0)
		n2 *= -1.f;

	vec4f n3 = vec4f::cross(simplex[3].p - simplex[2].p, simplex[1].p - simplex[2].p).getNormal();
	if (vec4f::dot(n3, simplex[0].p - simplex[2].p) > 0)
		n3 *= -1.f;

	vec4f n4 = vec4f::cross(simplex[3].p - simplex[2].p, simplex[0].p - simplex[2].p).getNormal();
	if (vec4f::dot(n4, simplex[1].p - simplex[2].p) > 0)
		n4 *= -1.f;

	// faces
	Face* f1 = &(*faces.insert(faces.end(), Face(simplex[0], simplex[1], simplex[2], n1)));
	Face* f2 = &(*faces.insert(faces.end(), Face(simplex[0], simplex[1], simplex[3], n2)));
	Face* f3 = &(*faces.insert(faces.end(), Face(simplex[1], simplex[2], simplex[3], n3)));
	Face* f4 = &(*faces.insert(faces.end(), Face(simplex[0], simplex[2], simplex[3], n4)));

	// edges
	Edge* e1 = &(*edges.insert(edges.end(), Edge(simplex[0], simplex[1])));
	Edge* e2 = &(*edges.insert(edges.end(), Edge(simplex[1], simplex[2])));
	Edge* e3 = &(*edges.insert(edges.end(), Edge(simplex[0], simplex[2])));
	Edge* e4 = &(*edges.insert(edges.end(), Edge(simplex[3], simplex[0])));
	Edge* e5 = &(*edges.insert(edges.end(), Edge(simplex[3], simplex[1])));
	Edge* e6 = &(*edges.insert(edges.end(), Edge(simplex[3], simplex[2])));

	// linking
	associate(f1, e1); associate(f1, e2); associate(f1, e3);
	associate(f2, e1); associate(f2, e4); associate(f2, e5);
	associate(f3, e2); associate(f3, e5); associate(f3, e6);
	associate(f4, e3); associate(f4, e4); associate(f4, e6);
}
bool GJK::GJKHull::add(const MinkowskiPoint& point)
{
	//	test if point inside current hull or already existing
	bool inside = true;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (vec4f::dot(it->n, point.p - it->p1.p) > EPSILON)
		{
			inside = false;
			break;
		}
	}
	if (inside)
		return true;
	if (!points.insert(point).second)
		return true;
	
	//	compute horizon
	for (auto it = edges.begin(); it != edges.end(); it++)
		it->horizonCheck = 2;
	for (auto it = faces.begin(); it != faces.end(); it++)
		it->onHull = true;
	std::list<Edge*> horizon = computeHorizon(point.p);

	//	compute cone faces
	for (auto it = horizon.begin(); it != horizon.end(); it++)
	{
		//	create cone face and add it to hull
		vec4f n = vec4f::cross((*it)->p2.p - (*it)->p1.p, point.p - (*it)->p1.p).getNormal();
		faces.insert(faces.end(), Face((*it)->p1, (*it)->p2, point, n));
		Face* face = &faces.back();
		if (checkFaceNormal(*face))
			face->n *= -1.f;

		//  create new horizon edge
		if (!(*it)->f1) 
			(*it)->f1 = face;
		else if (!(*it)->f2) 
			(*it)->f2 = face;
		else
		{
			if (verbose)
				std::cout << "GJKHull : Fatal error in horizon cone : both faces of an horizon edge are already set" << std::endl;
			return true;
		}
		face->e1 = (*it);

		// create others edges
		Edge* e2 = existingEdge((*it)->p1, point);
		if (!e2)
		{
			edges.insert(edges.end(), Edge((*it)->p1, point));
			e2 = &edges.back();
		}

		if (!e2->f1) 
			e2->f1 = face;
		else if (!e2->f2) 
			e2->f2 = face;
		else
		{
			if (verbose)
				std::cout << "GJKHull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			return true;
		}
		face->e2 = e2;

		Edge* e3 = existingEdge((*it)->p2, point);
		if (!e3)
		{
			edges.insert(edges.end(), Edge((*it)->p2, point));
			e3 = &edges.back();
		}

		if (!e3->f1) 
			e3->f1 = face;
		else if (!e3->f2) 
			e3->f2 = face;
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
GJK::Face* GJK::GJKHull::getClosestFaceToOrigin()
{
	float dmin = std::numeric_limits<float>::max();
	Face* face = &faces.front();

	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		float d = std::abs(vec4f::dot(it->n, -it->p1.p));
		if (d < dmin)
		{
			dmin = d;
			face = &(*it);
		}
	}
	return face;
}


void GJK::GJKHull::draw(const vec4f& offset)
{
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		vec4f p1 = it->p1.p + offset;
		vec4f p2 = it->p2.p + offset;
		vec4f p3 = it->p3.p + offset;

		Debug::color = Debug::magenta;
		Debug::drawLine(p1, p2);
		Debug::drawLine(p1, p3);
		Debug::drawLine(p3, p2);

		Debug::color = Debug::blue;
		Debug::drawLine(0.3333f*(p1 + p2 + p3), 0.3333f*(p1 + p2 + p3) + 0.15f * it->n);
	}
}


void GJK::GJKHull::associate(Face* f, Edge* e) const
{
	if (f->e1 == nullptr)
		f->e1 = e;
	else if (f->e2 == nullptr)
		f->e2 = e;
	else if (f->e3 == nullptr)
		f->e3 = e;
	else if (verbose)
		std::cout << "GJKHull : Fatal error ! : a face (triangle) has more than 3 edges" << std::endl;

	if (e->f1 == nullptr) 
		e->f1 = f;
	else if (e->f2 == nullptr) 
		e->f2 = f;
	else if (verbose)
		std::cout << "GJKHull : Fatal error ! : an edge has more than 2 faces" << std::endl;
}
GJK::Edge* GJK::GJKHull::existingEdge(const MinkowskiPoint& p1, const MinkowskiPoint& p2)
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
std::list<GJK::Edge*> GJK::GJKHull::computeHorizon(const vec4f& eye)
{
	std::list<Edge*> horizon;
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (vec4f::dot(it->n, eye - it->p1.p) >= 0)
		{
			it->onHull = false;

			it->e1->horizonCheck--;
			it->e2->horizonCheck--;
			it->e3->horizonCheck--;

			horizon.insert(horizon.end(), it->e1);
			horizon.insert(horizon.end(), it->e2);
			horizon.insert(horizon.end(), it->e3);

			if (it->e1->f1 == &(*it))
				it->e1->f1 = nullptr;
			else if (it->e1->f2 == &(*it))
				it->e1->f2 = nullptr;

			if (it->e2->f1 == &(*it)) 
				it->e2->f1 = nullptr;
			else if (it->e2->f2 == &(*it)) 
				it->e2->f2 = nullptr;

			if (it->e3->f1 == &(*it)) 
				it->e3->f1 = nullptr;
			else if (it->e3->f2 == &(*it)) 
				it->e3->f2 = nullptr;
		}
	}

	for (auto it = horizon.begin(); it != horizon.end(); )
	{
		if ((*it)->horizonCheck == 0) 
			it = horizon.erase(it);
		else 
			it++;
	}
	return horizon;
}
bool GJK::GJKHull::checkFaceNormal(const Face& f) const
{
	for (auto it = faces.begin(); it != faces.end(); it++)
	{
		if (vec4f::dot(f.n, it->p1.p - f.p1.p) > EPSILON)
			return true;
		else if (vec4f::dot(f.n, it->p2.p - f.p1.p) > EPSILON)
			return true;
		else if (vec4f::dot(f.n, it->p3.p - f.p1.p) > EPSILON)
			return true;
	}
	return false;
}
//