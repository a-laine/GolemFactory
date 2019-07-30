#pragma once

#include "Resources/Mesh.h"

#include <vector>
#include <set>

class IncrementalHull
{
	public:
		//	Miscellaneous
		struct Edge;
		struct Face
		{
			Face(const glm::vec3& point1 = glm::vec3(0.f), const glm::vec3& point2 = glm::vec3(0.f), const glm::vec3& point3 = glm::vec3(0.f), const glm::vec3& normal = glm::vec3(0.f)) :
				p1(point1), p2(point2), p3(point3), n(normal), onHull(true), e1(nullptr), e2(nullptr), e3(nullptr) {};

			glm::vec3 n;
			glm::vec3 p1, p2, p3;
			bool onHull;
			Edge *e1, *e2, *e3;
		};
		struct Edge
		{
			Edge(const glm::vec3& point1 = glm::vec3(0.f), const glm::vec3& point2 = glm::vec3(0.f)) : p1(point1), p2(point2), horizonCheck(2), f1(nullptr), f2(nullptr) {};

			glm::vec3 p1, p2;
			int horizonCheck;
			Face *f1, *f2;
		};
		//
		
		//  Default
		IncrementalHull();
		//

		//	Public functions
		Mesh* getConvexHull(Mesh* m);
		//Mesh* optimizeHullMesh(Mesh* mesh);
		//

	protected:
		//	Protected functions
		void initializeHull(const std::vector<glm::vec3>& pointCloud);
		std::list<Edge*> computeHorizon(const glm::vec3& eye);
		bool isFaceEdge(const Face& f, const Edge& e);
		Edge* existingEdge(const glm::vec3& p1, const glm::vec3& p2);
		bool checkFaceNormal(const Face& f);
		//

		//	Attributes
		bool degenerated;
		std::list<Edge> hullEdges;
		std::list<Face> hullFaces;
		//
};



