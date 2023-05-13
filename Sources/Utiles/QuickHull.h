#pragma once

#include <Resources/Mesh.h>

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
			Face(const vec4f& point1, const vec4f& point2, const vec4f& point3, const vec4f& normal) : 
				p1(point1), p2(point2), p3(point3), n(normal), onHull(true), e1(nullptr), e2(nullptr), e3(nullptr) {};

			vec4f n;
			vec4f p1, p2, p3;
			std::vector<vec4f> outter;
			bool onHull;
			Edge *e1, *e2, *e3;
		};
		struct Edge
		{
			Edge() : p1(0.f), p2(0.f), onHull(true), f1(nullptr), f2(nullptr) {};
			Edge(const vec4f& point1, const vec4f& point2) : p1(point1), p2(point2), onHull(true), f1(nullptr), f2(nullptr) {};

			vec4f p1, p2;
			bool onHull;
			Face *f1, *f2;
		};


	protected:
		void initializeHull(const std::vector<vec4f>& pointCloud);
		void computeHorizon(const vec4f& eye, Edge* crossedEdge, Face* face, std::list<Edge*>& horizon, std::vector<vec4f>& unclaimed);
		bool isFaceEdge(const Face& f, const Edge& e);
		Edge* existingEdge(const vec4f& p1, const vec4f& p2);

		bool degenerated;
		std::list<Edge> hullEdges;
		std::list<Face> hullFaces;
};
