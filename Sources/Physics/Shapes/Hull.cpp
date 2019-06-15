#include "Hull.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include "Resources/ResourceManager.h"
#include "Resources/Mesh.h"
#include "Utiles/ToolBox.h"

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

//	Default
//Hull::Hull(const std::vector<glm::vec3>& v, const std::vector<unsigned short>& f) : Shape(HULL), vertices(v), faces(f), degenerate(false){}
Hull::Hull(Mesh* m, glm::mat4 transform) : Shape(HULL), base(transform), mesh(m)
{}
//

//	Public functions
Sphere Hull::toSphere() const { return toAxisAlignedBox().toSphere(); }
AxisAlignedBox Hull::toAxisAlignedBox() const
{
	glm::vec3 max(std::numeric_limits<float>::min());
	glm::vec3 min(std::numeric_limits<float>::max());
	auto vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].x > max.x) max.x = vertices[i].x;
		if (vertices[i].y > max.y) max.y = vertices[i].y;
		if (vertices[i].z > max.z) max.z = vertices[i].z;

		if (vertices[i].x < min.x) min.x = vertices[i].x;
		if (vertices[i].y < min.y) min.y = vertices[i].y;
		if (vertices[i].z < min.z) min.z = vertices[i].z;
	}
	glm::vec3 p1 = glm::vec3(base*glm::vec4(min, 1.f));
	glm::vec3 p2 = glm::vec3(base*glm::vec4(max, 1.f));
	return AxisAlignedBox(glm::min(p1, p2), glm::max(p1, p2));
}
void Hull::operator=(const Shape& s)
{
	if (s.type == Shape::HULL)
	{
		const Hull& h = *static_cast<const Hull*>(&s);
		mesh = h.mesh;
		base = h.base;
	}
}
void Hull::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	base = m * base;
	//auto vertices = *mesh->getVertices();
	//for (unsigned int i = 0; i < vertices.size(); i++)
	//	vertices[i] = glm::vec3(m * glm::vec4(vertices[i], 1.f));
}
Shape* Hull::duplicate() const
{
	ResourceManager::getInstance()->getResource(mesh);
	return new Hull(mesh, base);
}
glm::vec3 Hull::GJKsupport(const glm::vec3& direction) const
{
	glm::vec3 u = glm::vec3(glm::inverse(base) * glm::vec4(direction, 1.f));
	float d = std::numeric_limits<float>::min();
	glm::vec3 v = glm::vec3(0.f);
	auto vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		float a = glm::dot(vertices[i], u);
		if (a > d)
		{
			d = a;
			v = vertices[i];
		}
	}
	return v;
}





// http://algolist.manual.ru/maths/geom/convhull/qhull3d.php 


struct vec3
{
	vec3(glm::vec3 v) :x(v.x), y(v.y), z(v.z) {}
	friend bool operator<(const vec3& l, const vec3& r)
	{
		if (l.x != r.x) return l.x < r.x;
		else if (l.y != r.y) return l.y < r.y;
		else return l.z < r.z;
	}
	float x, y, z;
};
struct Face;
struct Edge
{
	Edge() : onHull(true), p1(0.f), p2(0.f), f1(nullptr), f2(nullptr) {};
	Edge(const glm::vec3& point1, const glm::vec3& point2) : onHull(true), p1(point1), p2(point2), f1(nullptr), f2(nullptr) {};
	bool onHull;
	glm::vec3 p1, p2;
	Face *f1, *f2;
};
struct Face
{
	Face() : onHull(true), p1(0.f), p2(0.f), p3(0.f), e1(nullptr), e2(nullptr), e3(nullptr) {};
	Face(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3) : onHull(true), p1(point1), p2(point2), p3(point3), e1(nullptr), e2(nullptr), e3(nullptr) {};
	bool onHull;
	glm::vec3 n;
	glm::vec3 p1, p2, p3;
	Edge *e1, *e2, *e3;
	std::vector<glm::vec3> outter;
};
struct HullMesh
{
	bool degenerated;
	std::list<Edge> edges;
	std::list<Face> faces;

};


HullMesh quickhull(const Mesh* mesh);
HullMesh computeInitialPolyhedron(const std::vector<glm::vec3>& pointCloud);
void splitCloud(const std::vector<glm::vec3>& pointCloud, std::list<Face>& faces);
void splitCloud(const std::vector<glm::vec3>& pointCloud, std::list<Face*>& faces);
std::pair<glm::vec3, glm::vec3> initialSegment(const std::vector<glm::vec3>& cloud, HullMesh& hull);
bool isFaceEdge(const Face& f, const Edge& e);
Edge* existingEdge(const glm::vec3& p1, const glm::vec3& p2, HullMesh& hull);
bool existFacesOutterPoint(HullMesh& hull);
Face* getFaceWithOutter(HullMesh& hull);
void computeHorizon(const glm::vec3& eye, Edge* crossedEdge, Face* startFace, std::list<Edge*>& horizon, std::vector<glm::vec3>& unclaimed);

/*
Mesh* Hull::fromMesh(const Mesh* m)
{
	std::cout << "Hull creation from mesh : " << m->name << std::endl;

	HullMesh hull = quickhull(m);
	Mesh* mesh = new Mesh("hull_" + m->name);
	if (hull.degenerated)
	{
		std::cout << "  degenerated mesh !" << std::endl;
		mesh->initialize(std::vector<glm::vec3>(), std::vector<glm::vec3>(), std::vector<glm::vec3>(), std::vector<unsigned short>(), std::vector<glm::ivec3>(), std::vector<glm::vec3>());
	}
	else
	{
		std::cout << "  good mesh" << std::endl;
		glm::vec3 hullColor = glm::vec3(0.5f, 0.f, 1.f);
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> colors;
		std::vector<unsigned short> faces;

		for (auto it = hull.faces.begin(); it != hull.faces.end(); it++)
		{
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p1);
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p2);
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p3);

			colors.push_back(hullColor);
			colors.push_back(hullColor);
			colors.push_back(hullColor);

			normales.push_back(glm::normalize(it->n));
			normales.push_back(normales.back());
			normales.push_back(normales.back());
		}
		ToolBox::optimizeStaticMesh(vertices, normales, colors, faces);
		mesh->initialize(vertices, normales, colors, faces, std::vector<glm::ivec3>(), std::vector<glm::vec3>());
	}
	return mesh;
}
//

//	Generation algorithm
HullMesh quickhull(const Mesh* mesh)
{
	const std::vector<glm::vec3>& cloud = *mesh->getVertices();
	HullMesh hull = computeInitialPolyhedron(cloud);
	splitCloud(cloud, hull.faces);

	while (existFacesOutterPoint(hull))
	{
		//	prepare hull and aliases
		for (auto it = hull.edges.begin(); it != hull.edges.end(); it++)
			it->onHull = true;
		for (auto it = hull.faces.begin(); it != hull.faces.end(); it++)
			it->onHull = true;
		Face* f = getFaceWithOutter(hull);
		if (!f)
		{
			std::cout << "Quickhull : Fatal error in face iteration : ???" << std::endl;
			return hull;
		}

		// compute farest point
		glm::vec3 eye = f->outter[0];
		float maxd = std::numeric_limits<float>::min();
		for (unsigned int i = 0; i < f->outter.size(); i++)
		{
			float d = glm::dot(f->n, f->outter[i] - f->p1);
			if (d > maxd)
			{
				maxd = d;
				eye = f->outter[i];
			}
		}

		//	compute horizon
		std::list<Edge*> horizon;
		std::vector<glm::vec3> unclaimedPoints;
		computeHorizon(eye, nullptr, f, horizon, unclaimedPoints);

		if (horizon.empty())
		{
			std::cout << "horizon empty, eye : [" << eye.x << ' ' << eye.y << ' ' << eye.z << ']' << std::endl;
			f->outter.clear();
			continue;
		}
		else std::cout << "horizon : " << horizon.size() << " ; eye : [" << eye.x << ' ' << eye.y << ' ' << eye.z << ']' << std::endl;

		//	construct cone
		std::list<Face*> coneFaces;
		for (auto it = horizon.begin(); it != horizon.end(); it++)
		{
			std::cout << "    [" << (*it)->p1.x << ' ' << (*it)->p1.y << ' ' << (*it)->p1.z << "], [" << (*it)->p2.x << ' ' << (*it)->p2.y << ' ' << (*it)->p2.z << ']' << std::endl;

			//  create tmp face
			Face tmp((*it)->p1, (*it)->p2, eye);
			tmp.n = glm::cross(tmp.p2 - tmp.p1, tmp.p3 - tmp.p1);
			if (glm::dot(tmp.n, f->n) < 0) tmp.n *= -1.f;

			Face* otherFace = nullptr;
			if ((*it)->f1->onHull) otherFace = (*it)->f1;
			else if ((*it)->f2->onHull) otherFace = (*it)->f2;
			else std::cout << "Quickhull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			if (!otherFace) continue;

			//  detect weird case
			if (glm::cross(otherFace->n, tmp.n) == glm::vec3(0.f))
			{
				//return hull;
				//(*it)->onHull = false;
				std::cout << "WEIRD CASE" << std::endl;
			}


			//	create cone face and add it to hull
			hull.faces.insert(hull.faces.end(), Face((*it)->p1, (*it)->p2, eye));
			Face* face = &hull.faces.back();
			face->n = tmp.n;
			coneFaces.push_back(face);

			//  create new horizon edge
			hull.edges.insert(hull.edges.end(), Edge((*it)->p1, (*it)->p2));
			Edge* e1 = &hull.edges.back();
			if ((*it)->f1->onHull) e1->f1 = (*it)->f1;
			else if ((*it)->f2->onHull) e1->f1 = (*it)->f2;
			else std::cout << "Quickhull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			e1->f2 = face;
			face->e1 = e1;

			// create others edges
			Edge* e2 = existingEdge((*it)->p1, eye, hull);
			if (!e2)
			{
				hull.edges.insert(hull.edges.end(), Edge((*it)->p1, eye));
				e2 = &hull.edges.back();
			}
			if (!e2->f1) e2->f1 = face;
			else if (!e2->f2) e2->f2 = face;
			else std::cout << "Quickhull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			face->e2 = e2;

			Edge* e3 = existingEdge((*it)->p2, eye, hull);
			if (!e3)
			{
				hull.edges.insert(hull.edges.end(), Edge((*it)->p2, eye));
				e3 = &hull.edges.back();
			}
			if (!e3->f1) e3->f1 = face;
			else if (!e3->f2) e3->f2 = face;
			else std::cout << "Quickhull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			face->e3 = e3;
		}

		//	split unclaimed and clear hull from dead elements
		splitCloud(unclaimedPoints, coneFaces);
		for (auto it = hull.edges.begin(); it != hull.edges.end();)
		{
			if (it->onHull) it++;
			else it = hull.edges.erase(it);
		}
		for (auto it = hull.faces.begin(); it != hull.faces.end();)
		{
			if (it->onHull) it++;
			else it = hull.faces.erase(it);
		}
	}
	
	return hull;
}
HullMesh computeInitialPolyhedron(const std::vector<glm::vec3>& pointCloud)
{
	HullMesh hull;
	std::pair<glm::vec3, glm::vec3> s = initialSegment(pointCloud, hull);
	if (hull.degenerated) return hull;

	// search maximum distant point from initial segment
	float maxd = std::numeric_limits<float>::min();
	glm::vec3 T;
	glm::vec3 u = s.second - s.first;
	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		float d = glm::length(glm::cross(pointCloud[j] - s.first, u));
		if (d > maxd)
		{
			maxd = d;
			T = pointCloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 1D
	{
		hull.degenerated = true;
		return hull;
	}

	// search maximum distant point from triangle
	maxd = std::numeric_limits<float>::min();
	glm::vec3 P;
	glm::vec3 n = glm::cross(T - s.first, u);
	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		float d = std::abs(glm::dot(pointCloud[j], n));
		if (d > maxd)
		{
			maxd = d;
			P = pointCloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 2D
	{
		hull.degenerated = true;
		return hull;
	}

	// construct tetrahedron
	hull.edges.push_back(Edge(s.first, s.second));
	hull.edges.push_back(Edge(s.first, T));
	hull.edges.push_back(Edge(s.first, P));
	hull.edges.push_back(Edge(s.second, T));
	hull.edges.push_back(Edge(s.second, P));
	hull.edges.push_back(Edge(T, P));

	Face f1(s.first, s.second, T);
	f1.n = glm::cross(u, T - s.first);
	if (glm::dot(f1.n, P) > 0) f1.n *= -1.f;
	hull.faces.push_back(f1);

	Face f2(s.first, s.second, P);
	f2.n = glm::cross(u, P - s.first);
	if (glm::dot(f2.n, T) > 0) f2.n *= -1.f;
	hull.faces.push_back(f2);

	Face f3(s.first, T, P);
	f3.n = glm::cross(T - s.first, P - s.first);
	if (glm::dot(f3.n, s.second) > 0) f3.n *= -1.f;
	hull.faces.push_back(f3);

	Face f4(s.second, T, P);
	f4.n = glm::cross(T - s.second, P - s.second);
	if (glm::dot(f4.n, s.first) > 0) f4.n *= -1.f;
	hull.faces.push_back(f4);

	// assign references
	for (auto it = hull.faces.begin(); it != hull.faces.end(); it++)
	{
		for (auto it2 = hull.edges.begin(); it2 != hull.edges.end(); it2++)
		{
			if (isFaceEdge(*it, *it2))
			{
				if (it->e1 == nullptr) it->e1 = &(*it2);
				else if (it->e2 == nullptr) it->e2 = &(*it2);
				else if (it->e3 == nullptr) it->e3 = &(*it2);
				else std::cout << "Quickhull : Fatal error ! : a face (triangle) has more than 3 edges" << std::endl;

				if (it2->f1 == nullptr) it2->f1 = &(*it);
				else if (it2->f2 == nullptr) it2->f2 = &(*it);
				else std::cout << "Quickhull : Fatal error ! : an edge has more than 2 faces" << std::endl;
			}
			if (it->e3 != nullptr)
				break;
		}
	}

	return hull;
}
void splitCloud(const std::vector<glm::vec3>& pointCloud, std::list<Face>& faces)
{
	for (unsigned int i = 0; i < pointCloud.size(); i++)
	{
		for (auto it = faces.begin(); it != faces.end(); it++)
		{
			if (glm::dot(it->n, pointCloud[i] - it->p1) > 0)
			{
				it->outter.push_back(pointCloud[i]);
				break;
			}
		}
	}
}
void splitCloud(const std::vector<glm::vec3>& pointCloud, std::list<Face*>& faces)
{
	int placed = 0;
	for (unsigned int i = 0; i < pointCloud.size(); i++)
	{
		for (auto it = faces.begin(); it != faces.end(); it++)
		{
			if (glm::dot((*it)->n, pointCloud[i] - (*it)->p1) > 0)
			{
				(*it)->outter.push_back(pointCloud[i]);
				placed++;
				break;
			}
		}
	}
	if (placed)
		std::cout << "placed vertices : " << placed << std::endl;
}
void computeHorizon(const glm::vec3& eye, Edge* crossedEdge, Face* face, std::list<Edge*>& horizon, std::vector<glm::vec3>& unclaimed)
{
	if (!face->onHull)
	{
		crossedEdge->onHull = false;
		return;
	}

	if (glm::dot(face->n, eye - face->p1) > 0)
	{
		face->onHull = false;
		unclaimed.insert(unclaimed.end(), face->outter.begin(), face->outter.end());
		face->outter.clear();
		if (crossedEdge)
			crossedEdge->onHull = false;

		if (face->e1->onHull)
		{
			if (face->e1->f1->onHull)
				computeHorizon(eye, face->e1, face->e1->f1, horizon, unclaimed);
			if (face->e1->f2->onHull)
				computeHorizon(eye, face->e1, face->e1->f2, horizon, unclaimed);
		}
		if (face->e2->onHull)
		{
			if (face->e2->f1->onHull)
				computeHorizon(eye, face->e2, face->e2->f1, horizon, unclaimed);
			if (face->e2->f2->onHull)
				computeHorizon(eye, face->e2, face->e2->f2, horizon, unclaimed);
		}
		if (face->e3->onHull)
		{
			if (face->e3->f1->onHull)
				computeHorizon(eye, face->e3, face->e3->f1, horizon, unclaimed);
			if (face->e3->f2->onHull)
				computeHorizon(eye, face->e3, face->e3->f2, horizon, unclaimed);
		}
	}
	else if(crossedEdge)
	{
		crossedEdge->onHull = false;
		horizon.push_back(crossedEdge);
	}
}


std::pair<glm::vec3, glm::vec3> initialSegment(const std::vector<glm::vec3>& cloud, HullMesh& hull)
{
	hull.degenerated = false;

	// compute maximum point on principal axis
	float minx = std::numeric_limits<float>::max();     glm::vec3 x;
	float maxx = std::numeric_limits<float>::min();     glm::vec3 X;
	float miny = std::numeric_limits<float>::max();		glm::vec3 y;
	float maxy = std::numeric_limits<float>::min();		glm::vec3 Y;
	float minz = std::numeric_limits<float>::max();		glm::vec3 z;
	float maxz = std::numeric_limits<float>::min();		glm::vec3 Z;

	for (unsigned int j = 0; j < cloud.size(); j++)
	{
		if (cloud[j].x < minx)
		{
			minx = cloud[j].x;
			x = cloud[j];
		}
		if (cloud[j].x > maxx)
		{
			maxx = cloud[j].x;
			X = cloud[j];
		}

		if (cloud[j].y < miny)
		{
			miny = cloud[j].y;
			y = cloud[j];
		}
		if (cloud[j].y > maxy)
		{
			maxy = cloud[j].y;
			Y = cloud[j];
		}

		if (cloud[j].z < minz)
		{
			minz = cloud[j].z;
			z = cloud[j];
		}
		if (cloud[j].z > maxz)
		{
			maxz = cloud[j].z;
			Z = cloud[j];
		}
	}

	if (x == X) hull.degenerated = true;
	else if (y == Y) hull.degenerated = true;
	else if (z == Z) hull.degenerated = true;

	if (hull.degenerated)
		return std::pair<glm::vec3, glm::vec3>(glm::vec3(0.f), glm::vec3(0.f));

	float d1 = maxx - minx;
	float d2 = maxy - miny;
	float d3 = maxz - minz;
	if (d1 > d2 && d1 > d3) return std::pair<glm::vec3, glm::vec3>(x, X);
	else if (d2 > d1 && d2 > d3) return std::pair<glm::vec3, glm::vec3>(y, Y);
	else return std::pair<glm::vec3, glm::vec3>(z, Z);
}
bool isFaceEdge(const Face& f, const Edge& e)
{
	if (f.p1 == e.p1 && f.p2 == e.p2) return true;			// just f.p3 is not in edge
	else if (f.p2 == e.p1 && f.p3 == e.p2) return true;		// just f.p1 is not in edge
	else if (f.p1 == e.p1 && f.p3 == e.p2) return true;		// just f.p2 is not in edge
	else if (f.p1 == e.p2 && f.p2 == e.p1) return true;			// just f.p3 is not in edge
	else if (f.p2 == e.p2 && f.p3 == e.p1) return true;		// just f.p1 is not in edge
	else if (f.p1 == e.p2 && f.p3 == e.p1) return true;		// just f.p2 is not in edge
	else return false;
}
Edge* existingEdge(const glm::vec3& p1, const glm::vec3& p2, HullMesh& hull)
{
	for (auto it = hull.edges.begin(); it != hull.edges.end(); it++)
	{
		if (it->p1 == p1 && it->p2 == p2)
			return &(*it);
		else if (it->p1 == p2 && it->p2 == p1)
			return &(*it);
	}
	return nullptr;
}
bool existFacesOutterPoint(HullMesh& hull)
{
	return getFaceWithOutter(hull) != nullptr;
}
Face* getFaceWithOutter(HullMesh& hull)
{
	for (auto it = hull.faces.begin(); it != hull.faces.end(); it++)
	{
		if (it->outter.size() != 0)
			return &(*it);
	}
	return nullptr;
}
*/
//