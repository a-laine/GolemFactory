#include "QuickHull.h"
#include "Utiles/ToolBox.h"


QuickHull::QuickHull() : degenerated(true) {};
Mesh* QuickHull::getConvexHull(Mesh* m)
{
	// initialization
	std::cout << "Hull creation from mesh : " << m->name << std::endl;
	const std::vector<glm::vec3>& pointCloud = *m->getVertices();
	initializeHull(pointCloud);
	std::set<Face*> faceStack;
	for (unsigned int i = 0; i < pointCloud.size(); i++)
	{
		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
		{
			if (glm::dot(it->n, pointCloud[i] - it->p1) > 0)
			{
				it->outter.push_back(pointCloud[i]);
				faceStack.insert(&(*it));
				break;
			}
		}
	}

	//	iterate
	while (!faceStack.empty())
	{
		//	prepare hull and aliases
		for (auto it = hullEdges.begin(); it != hullEdges.end(); it++)
			it->onHull = true;
		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
			it->onHull = true;
		Face* f = *faceStack.begin();
		if (!f)
		{
			std::cout << "Quickhull : Fatal error in face iteration : ???" << std::endl;
			break;
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
			Face tmp((*it)->p1, (*it)->p2, eye, glm::cross((*it)->p2 - (*it)->p1, eye - (*it)->p1));
			if (glm::dot(tmp.n, f->n) < 0) tmp.n *= -1.f;

			Face* otherFace = nullptr;
			if ((*it)->f1->onHull) otherFace = (*it)->f1;
			else if ((*it)->f2->onHull) otherFace = (*it)->f2;
			else std::cout << "Quickhull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			if (!otherFace) continue;

			//  detect weird case
			if (glm::cross(otherFace->n, tmp.n) == glm::vec3(0.f))
			{
				std::cout << "WEIRD CASE" << std::endl;
			}


			//	create cone face and add it to hull
			hullFaces.insert(hullFaces.end(), Face((*it)->p1, (*it)->p2, eye, tmp.n));
			Face* face = &hullFaces.back();
			coneFaces.push_back(face);

			//  create new horizon edge
			hullEdges.insert(hullEdges.end(), Edge((*it)->p1, (*it)->p2));
			Edge* e1 = &hullEdges.back();
			if ((*it)->f1->onHull) e1->f1 = (*it)->f1;
			else if ((*it)->f2->onHull) e1->f1 = (*it)->f2;
			else std::cout << "Quickhull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			e1->f2 = face;
			face->e1 = e1;

			// create others edges
			Edge* e2 = existingEdge((*it)->p1, eye);
			if (!e2)
			{
				hullEdges.insert(hullEdges.end(), Edge((*it)->p1, eye));
				e2 = &hullEdges.back();
			}
			if (!e2->f1) e2->f1 = face;
			else if (!e2->f2) e2->f2 = face;
			else std::cout << "Quickhull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			face->e2 = e2;

			Edge* e3 = existingEdge((*it)->p2, eye);
			if (!e3)
			{
				hullEdges.insert(hullEdges.end(), Edge((*it)->p2, eye));
				e3 = &hullEdges.back();
			}
			if (!e3->f1) e3->f1 = face;
			else if (!e3->f2) e3->f2 = face;
			else std::cout << "Quickhull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			face->e3 = e3;
		}

		//	split unclaimed and clear hull from dead elements
		for (unsigned int i = 0; i < unclaimedPoints.size(); i++)
		{
			for (auto it = coneFaces.begin(); it != coneFaces.end(); it++)
			{
				if (glm::dot((*it)->n, unclaimedPoints[i] - (*it)->p1) > 0)
				{
					(*it)->outter.push_back(unclaimedPoints[i]);
					break;
				}
			}
		}
		for (auto it = hullEdges.begin(); it != hullEdges.end();)
		{
			if (it->onHull) it++;
			else it = hullEdges.erase(it);
		}
		for (auto it = hullFaces.begin(); it != hullFaces.end();)
		{
			if (it->onHull) it++;
			else it = hullFaces.erase(it);
		}
	}

	//	prepare mesh from hull data
	Mesh* mesh = new Mesh("hull_" + m->name);
	if (degenerated)
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

		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
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


void QuickHull::initializeHull(const std::vector<glm::vec3>& pointCloud)
{
	//	compute initial segment
	glm::vec3 p1, p2;
	float minx = std::numeric_limits<float>::max();     glm::vec3 x;
	float maxx = std::numeric_limits<float>::min();     glm::vec3 X;
	float miny = std::numeric_limits<float>::max();		glm::vec3 y;
	float maxy = std::numeric_limits<float>::min();		glm::vec3 Y;
	float minz = std::numeric_limits<float>::max();		glm::vec3 z;
	float maxz = std::numeric_limits<float>::min();		glm::vec3 Z;

	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		if (pointCloud[j].x < minx)
		{
			minx = pointCloud[j].x;
			x = pointCloud[j];
		}
		if (pointCloud[j].x > maxx)
		{
			maxx = pointCloud[j].x;
			X = pointCloud[j];
		}

		if (pointCloud[j].y < miny)
		{
			miny = pointCloud[j].y;
			y = pointCloud[j];
		}
		if (pointCloud[j].y > maxy)
		{
			maxy = pointCloud[j].y;
			Y = pointCloud[j];
		}

		if (pointCloud[j].z < minz)
		{
			minz = pointCloud[j].z;
			z = pointCloud[j];
		}
		if (pointCloud[j].z > maxz)
		{
			maxz = pointCloud[j].z;
			Z = pointCloud[j];
		}
	}

	if (x == X) degenerated = true;
	else if (y == Y) degenerated = true;
	else if (z == Z) degenerated = true;
	if (degenerated) return;

	float d1 = maxx - minx;
	float d2 = maxy - miny;
	float d3 = maxz - minz;
	if (d1 > d2 && d1 > d3) { p1 = x, p2 = X; }
	else if (d2 > d1 && d2 > d3) { p1 = y, p2 = Y; }
	else { p1 = z, p2 = Z; };

	// search maximum distant point from initial segment
	float maxd = std::numeric_limits<float>::min();
	glm::vec3 T;
	glm::vec3 u = p2 - p1;
	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		float d = glm::length(glm::cross(pointCloud[j] -p1, u));
		if (d > maxd)
		{
			maxd = d;
			T = pointCloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 1D
	{
		degenerated = true;
		return;
	}

	// search maximum distant point from triangle
	maxd = std::numeric_limits<float>::min();
	glm::vec3 P;
	glm::vec3 n = glm::cross(T - p1, u);
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
		degenerated = true;
		return;
	}


	// construct tetrahedron
	hullEdges.push_back(Edge(p1, p2));
	hullEdges.push_back(Edge(p1, T));
	hullEdges.push_back(Edge(p1, P));
	hullEdges.push_back(Edge(p2, T));
	hullEdges.push_back(Edge(p2, P));
	hullEdges.push_back(Edge(T, P));

	Face f1(p1, p2, T, glm::cross(u, T - p1));
	if (glm::dot(f1.n, P) > 0) f1.n *= -1.f;
	hullFaces.push_back(f1);

	Face f2(p1, p2, P, glm::cross(u, P - p1));
	if (glm::dot(f2.n, T) > 0) f2.n *= -1.f;
	hullFaces.push_back(f2);

	Face f3(p1, T, P, glm::cross(T - p1, P - p1));
	if (glm::dot(f3.n, p2) > 0) f3.n *= -1.f;
	hullFaces.push_back(f3);

	Face f4(p2, T, P, glm::cross(T - p2, P - p2));
	if (glm::dot(f4.n, p1) > 0) f4.n *= -1.f;
	hullFaces.push_back(f4);

	// assign pointers
	for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
	{
		for (auto it2 = hullEdges.begin(); it2 != hullEdges.end(); it2++)
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

}
void QuickHull::computeHorizon(const glm::vec3& eye, Edge* crossedEdge, Face* face, std::list<Edge*>& horizon, std::vector<glm::vec3>& unclaimed)
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
	else if (crossedEdge)
	{
		crossedEdge->onHull = false;
		horizon.push_back(crossedEdge);
	}
}
bool QuickHull::isFaceEdge(const Face& f, const Edge& e)
{
	if (f.p1 == e.p1 && f.p2 == e.p2) return true;			// just f.p3 is not in edge
	else if (f.p2 == e.p1 && f.p3 == e.p2) return true;		// just f.p1 is not in edge
	else if (f.p1 == e.p1 && f.p3 == e.p2) return true;		// just f.p2 is not in edge
	else if (f.p1 == e.p2 && f.p2 == e.p1) return true;			// just f.p3 is not in edge
	else if (f.p2 == e.p2 && f.p3 == e.p1) return true;		// just f.p1 is not in edge
	else if (f.p1 == e.p2 && f.p3 == e.p1) return true;		// just f.p2 is not in edge
	else return false;
}
QuickHull::Edge* QuickHull::existingEdge(const glm::vec3& p1, const glm::vec3& p2)
{
	for (auto it = hullEdges.begin(); it != hullEdges.end(); it++)
	{
		if (it->p1 == p1 && it->p2 == p2)
			return &(*it);
		else if (it->p1 == p2 && it->p2 == p1)
			return &(*it);
	}
	return nullptr;
}

