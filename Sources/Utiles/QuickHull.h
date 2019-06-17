#pragma once

#include "Resources/Mesh.h"

#include <vector>
#include <set>

class QuickHull
{
	public:
		QuickHull();
		Mesh* getConvexHull(Mesh* m);

		
		struct Edge;
		struct Face
		{
			Face() : p1(0.f), p2(0.f), p3(0.f), n(0.f), onHull(true), e1(nullptr), e2(nullptr), e3(nullptr) {};
			Face(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3, const glm::vec3& normal) : 
				p1(point1), p2(point2), p3(point3), n(normal), onHull(true), e1(nullptr), e2(nullptr), e3(nullptr) {};

			glm::vec3 n;
			glm::vec3 p1, p2, p3;
			std::vector<glm::vec3> outter;
			bool onHull;
			Edge *e1, *e2, *e3;
		};
		struct Edge
		{
			Edge() : p1(0.f), p2(0.f), onHull(true), f1(nullptr), f2(nullptr) {};
			Edge(const glm::vec3& point1, const glm::vec3& point2) : p1(point1), p2(point2), onHull(true), f1(nullptr), f2(nullptr) {};

			glm::vec3 p1, p2;
			bool onHull;
			Face *f1, *f2;
		};


	protected:
		void initializeHull(const std::vector<glm::vec3>& pointCloud);
		void computeHorizon(const glm::vec3& eye, Edge* crossedEdge, Face* face, std::list<Edge*>& horizon, std::vector<glm::vec3>& unclaimed);
		bool isFaceEdge(const Face& f, const Edge& e);
		Edge* existingEdge(const glm::vec3& p1, const glm::vec3& p2);

		bool degenerated;
		std::list<Edge> hullEdges;
		std::list<Face> hullFaces;
};
