#pragma once

#include <Resources/Mesh.h>

#include <vector>
#include <set>

class IncrementalHull
{
	public:
		//	Miscellaneous
		struct Edge;
		struct Face
		{
			Face(const vec4f& point1 = vec4f(0.f), const vec4f& point2 = vec4f(0.f), const vec4f& point3 = vec4f(0.f), const vec4f& normal = vec4f(0.f)) :
				p1(point1), p2(point2), p3(point3), n(normal), onHull(true), e1(nullptr), e2(nullptr), e3(nullptr) {};

			vec4f n;
			vec4f p1, p2, p3;
			bool onHull;
			Edge *e1, *e2, *e3;
		};
		struct Edge
		{
			Edge(const vec4f& point1 = vec4f(0.f), const vec4f& point2 = vec4f(0.f)) : p1(point1), p2(point2), horizonCheck(2), f1(nullptr), f2(nullptr) {};

			vec4f p1, p2;
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
		void initializeHull(const std::vector<vec4f>& pointCloud);
		std::list<Edge*> computeHorizon(const vec4f& eye);
		bool isFaceEdge(const Face& f, const Edge& e);
		Edge* existingEdge(const vec4f& p1, const vec4f& p2);
		bool checkFaceNormal(const Face& f);
		//

		//	Attributes
		bool degenerated;
		std::list<Edge> hullEdges;
		std::list<Face> hullFaces;
		//
};



