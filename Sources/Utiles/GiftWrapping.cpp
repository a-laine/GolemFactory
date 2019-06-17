#include "GiftWrapping.h"
#include "Utiles/ToolBox.h"

#include <glm/gtx/vector_angle.hpp>


GiftWrapping::GiftWrapping() : degenerated(true) {}
Mesh* GiftWrapping::getConvexHull(Mesh* m)
{
	//	initialize
	std::cout << "Hull creation from mesh : " << m->name << std::endl;
	const std::vector<glm::vec3>& pointCloud = *m->getVertices();
	initializeHull(pointCloud);

	//	iterate through stack
	while(!edgeStack.empty())
	{
		//	pop one edge
		Edge e = edgeStack.back();
		edgeStack.pop_back();
		if (hullEdges.find(e) != hullEdges.end())
			continue;

		//	search other face
		float maxd = std::numeric_limits<float>::min();
		glm::vec3 p;
		glm::vec3 direction = -glm::cross(glm::cross(e.p2 - e.p1, e.firstFaceP3 - e.p1), e.p2 - e.p1);
		for (unsigned int i = 1; i < pointCloud.size(); i++)
		{
			float d = glm::dot(direction, pointCloud[i] - e.p1);
			if (d > maxd && pointCloud[i] != e.p1 && pointCloud[i] != e.p2)
			{
				if (glm::length2(glm::cross(e.p2 - e.p1, pointCloud[i] - e.p1)) > 0)	// avoid degenerated triangle
				{
					maxd = d;
					p = pointCloud[i];
				}
			}
		}
		
		//	create face
		glm::vec3 n = glm::cross(e.p2 - e.p1, p - e.p1);
		if (glm::dot(n, e.firstFaceP3 - e.p1) > 0) n *= -1.f;
		hullFaces.insert(Face(e.p1, e.p2, p, n));
		hullEdges.insert(e);
		edgeStack.push_back(Edge(e.p1, p, e.p2));
		edgeStack.push_back(Edge(e.p2, p, e.p1));
	}

	//	create and optimize mesh
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

void GiftWrapping::initializeHull(const std::vector<glm::vec3>& pointCloud)
{
	degenerated = (pointCloud.size() < 4);
	if (degenerated) return;

	float mind1 = std::numeric_limits<float>::max();
	float mind2 = std::numeric_limits<float>::max();
	float mind3 = std::numeric_limits<float>::max();
	float maxd = std::numeric_limits<float>::min();
	glm::vec3 p1, p2, p3, p4;

	//	search initial points
	for (unsigned int i = 0; i < pointCloud.size(); i++)
	{
		float d = pointCloud[i].z;
		if (d > maxd)
		{
			maxd = d;
			p4 = pointCloud[i];
		}

		if (d < mind1)
		{
			if (pointCloud[i] != p1)
			{
				mind3 = mind2;  p3 = p2;
				mind2 = mind1;  p2 = p1;
				mind1 = d;      p1 = pointCloud[i];
			}
		}
		else if (d < mind2)
		{
			if (pointCloud[i] != p2)
			{
				mind3 = mind2;  p3 = p2;
				mind2 = d;      p2 = pointCloud[i];
			}
		}
		else if (d < mind3)
		{
			if (pointCloud[i] != p3)
			{
				mind3 = d;
				p3 = pointCloud[i];
			}
		}
	}

	// test degenerated hull
	if (mind3 == std::numeric_limits<float>::max() || mind2 == std::numeric_limits<float>::max() || mind1 == std::numeric_limits<float>::max())
	{
		degenerated = true;
		return;
	}
	if(p1 == p2 || p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4 || p3 == p4)
	{
		degenerated = true;
		return;
	}

	//	create initial face
	glm::vec3 n = glm::cross(p2 - p1, p3 - p1);
	if (glm::dot(n, p4 - p1) > 0) n *= -1.f;
	hullFaces.insert(Face(p1, p2, p3, n));

	//	create initial edges
	Edge e1(p1, p2, p3);
	Edge e2(p2, p3, p1);
	Edge e3(p1, p3, p2);
	edgeStack.push_back(e1);
	edgeStack.push_back(e2);
	edgeStack.push_back(e3);
}
