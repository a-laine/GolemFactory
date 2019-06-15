#pragma once

#include "Resources/Mesh.h"

#include <vector>
#include <set>

class IncrementalHull
{
	public:
		IncrementalHull();
		Mesh* getConvexHull(Mesh* m);

		/*struct vec3
		{
			vec3(glm::vec3 v) :x(v.x), y(v.y), z(v.z) {}
			friend bool operator<(const vec3& l, const vec3& r)
			{
				if (l.x != r.x) return l.x < r.x;
				else if (l.y != r.y) return l.y < r.y;
				else return l.z < r.z;
			}
			glm::vec3 vec() const { return glm::vec3(x, y, z); }
			float x, y, z;
		};*/
		struct Edge;
		struct Face
		{
			Face() : p1(0.f), p2(0.f), p3(0.f), n(0.f), onHull(true) {};
			Face(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3, const glm::vec3& normal) : p1(point1), p2(point2), p3(point3), n(normal), onHull(true) {};

			glm::vec3 n;
			glm::vec3 p1, p2, p3;
			bool onHull;
			Edge *e1, *e2, *e3;
		};
		struct Edge
		{
			Edge() : p1(0.f), p2(0.f), onHull(true) {};
			Edge(const glm::vec3& point1, const glm::vec3& point2) : p1(point1), p2(point2), onHull(true) {};

			glm::vec3 p1, p2;
			bool onHull;
			Face *f1, *f2;
		};

	protected:
		void initializeHull(const std::vector<glm::vec3>& pointCloud);
		void computeHorizon(const glm::vec3& eye, Edge* crossedEdge, Face* face, std::list<Edge*>& horizon);
		Edge* existingEdge(const glm::vec3& p1, const glm::vec3& p2);

		bool degenerated;
		std::list<Edge> hullEdges;
		std::list<Face> hullFaces;
};



