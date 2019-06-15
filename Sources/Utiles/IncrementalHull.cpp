#include "IncrementalHull.h"
#include "Utiles/ToolBox.h"


IncrementalHull::IncrementalHull() {};
Mesh* IncrementalHull::getConvexHull(Mesh* m)
{
	//	initialization
	std::cout << "Hull creation from mesh : " << m->name << std::endl;
	std::vector<glm::vec3> pointCloud = *m->getVertices();
	initializeHull(pointCloud);

	//	iterate
	for(unsigned int i=0; i< pointCloud.size(); i++)
	{
		//	test if point inside current hull
		Face* f;
		bool inside = false;
		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
		{
			if (glm::dot(it->n, pointCloud[i] - it->p1) > 0)
			{
				inside = true;
				f = &(*it);
				break;
			}
		}
		if (inside)
			continue;

		//	compute horizon
		for (auto it = hullEdges.begin(); it != hullEdges.end(); it++)
			it->onHull = true;
		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
			it->onHull = true;
		std::list<Edge*> horizon;
		computeHorizon(pointCloud[i], nullptr, f, horizon);

		if (horizon.empty())
		{
			std::cout << "horizon empty, eye : [" << pointCloud[i].x << ' ' << pointCloud[i].y << ' ' << pointCloud[i].z << ']' << std::endl;
			continue;
		}
		else std::cout << "horizon : " << horizon.size() << " ; eye : [" << pointCloud[i].x << ' ' << pointCloud[i].y << ' ' << pointCloud[i].z << ']' << std::endl;

		//	compute cone faces
		for (auto it = horizon.begin(); it != horizon.end(); it++)
		{
			//  create tmp face
			Face tmp((*it)->p1, (*it)->p2, pointCloud[i], glm::cross((*it)->p2 - (*it)->p1, pointCloud[i] - (*it)->p1));
			if (glm::dot(tmp.n, f->n) < 0) tmp.n *= -1.f;

			Face* otherFace = nullptr;
			if ((*it)->f1->onHull) otherFace = (*it)->f1;
			else if ((*it)->f2->onHull) otherFace = (*it)->f2;
			else std::cout << "Quickhull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			if (!otherFace) continue;

			//	create cone face and add it to hull
			hullFaces.insert(hullFaces.end(), Face((*it)->p1, (*it)->p2, pointCloud[i], tmp.n));
			Face* face = &hullFaces.back();

			//  create new horizon edge
			hullEdges.insert(hullEdges.end(), Edge((*it)->p1, (*it)->p2));
			Edge* e1 = &hullEdges.back();
			if ((*it)->f1->onHull) e1->f1 = (*it)->f1;
			else if ((*it)->f2->onHull) e1->f1 = (*it)->f2;
			else std::cout << "Quickhull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			e1->f2 = face;
			face->e1 = e1;

			// create others edges
			Edge* e2 = existingEdge((*it)->p1, pointCloud[i]);
			if (!e2)
			{
				hullEdges.insert(hullEdges.end(), Edge((*it)->p1, pointCloud[i]));
				e2 = &hullEdges.back();
			}
			if (!e2->f1) e2->f1 = face;
			else if (!e2->f2) e2->f2 = face;
			else std::cout << "Quickhull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			face->e2 = e2;

			Edge* e3 = existingEdge((*it)->p2, pointCloud[i]);
			if (!e3)
			{
				hullEdges.insert(hullEdges.end(), Edge((*it)->p2, pointCloud[i]));
				e3 = &hullEdges.back();
			}
			if (!e3->f1) e3->f1 = face;
			else if (!e3->f2) e3->f2 = face;
			else std::cout << "Quickhull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			face->e3 = e3;
		}



		//	clear hull from dead entities
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

IncrementalHull::Edge* IncrementalHull::existingEdge(const glm::vec3& p1, const glm::vec3& p2)
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



