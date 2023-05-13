#include "GiftWrapping.h"
#include <Utiles/ToolBox.h>

//#include <glm/gtx/vector_angle.hpp>


GiftWrapping::GiftWrapping() : degenerated(true) {}
Mesh* GiftWrapping::getConvexHull(Mesh* m)
{
	//	initialize
	std::cout << "Hull creation from mesh : " << m->name << std::endl;
	const std::vector<vec4f>& pointCloud = *m->getVertices();
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
		vec4f p;
		vec4f direction = -vec4f::cross(vec4f::cross(e.p2 - e.p1, e.firstFaceP3 - e.p1), e.p2 - e.p1);
		for (unsigned int i = 1; i < pointCloud.size(); i++)
		{
			float d = vec4f::dot(direction, pointCloud[i] - e.p1);
			if (d > maxd && pointCloud[i] != e.p1 && pointCloud[i] != e.p2)
			{
				if (vec4f::cross(e.p2 - e.p1, pointCloud[i] - e.p1).getNorm2() > 0)	// avoid degenerated triangle
				{
					maxd = d;
					p = pointCloud[i];
				}
			}
		}
		
		//	create face
		vec4f n = vec4f::cross(e.p2 - e.p1, p - e.p1);
		if (vec4f::dot(n, e.firstFaceP3 - e.p1) > 0) n *= -1.f;
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
		mesh->initialize(std::vector<vec4f>(), std::vector<vec4f>(), std::vector<vec4f>(), std::vector<unsigned short>(), std::vector<vec4i>(), std::vector<vec4f>());
	}
	else
	{
		std::cout << "  good mesh" << std::endl;
		vec4f hullColor = vec4f(0.5f, 0.f, 1.f, 1.f);
		std::vector<vec4f> vertices;
		std::vector<vec4f> normales;
		std::vector<vec4f> colors;
		std::vector<unsigned short> faces;

		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
		{
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p1);
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p2);
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p3);

			colors.push_back(hullColor);
			colors.push_back(hullColor);
			colors.push_back(hullColor);

			normales.push_back(it->n.getNormal());
			normales.push_back(normales.back());
			normales.push_back(normales.back());
		}
		ToolBox::optimizeStaticMesh(vertices, normales, colors, faces);
		mesh->initialize(vertices, normales, colors, faces, std::vector<vec4i>(), std::vector<vec4f>());
	}
	return mesh;
}

void GiftWrapping::initializeHull(const std::vector<vec4f>& pointCloud)
{
	degenerated = (pointCloud.size() < 4);
	if (degenerated) return;

	float mind1 = std::numeric_limits<float>::max();
	float mind2 = std::numeric_limits<float>::max();
	float mind3 = std::numeric_limits<float>::max();
	float maxd = std::numeric_limits<float>::min();
	vec4f p1, p2, p3, p4;

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
	vec4f n = vec4f::cross(p2 - p1, p3 - p1);
	if (vec4f::dot(n, p4 - p1) > 0) n *= -1.f;
	hullFaces.insert(Face(p1, p2, p3, n));

	//	create initial edges
	Edge e1(p1, p2, p3);
	Edge e2(p2, p3, p1);
	Edge e3(p1, p3, p2);
	edgeStack.push_back(e1);
	edgeStack.push_back(e2);
	edgeStack.push_back(e3);
}
