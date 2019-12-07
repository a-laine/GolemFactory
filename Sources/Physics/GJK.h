#pragma once
#include "Shapes/Shape.h"
#include "SpecificIntersection/IntersectionContact.h"
#include "SpecificIntersection/IntersectionPoint.h"

#include <list>
#include <set>


class GJK
{
	public:
		//	Public functions
		static bool verbose;
		static bool gizmos;
		static int max_iteration;

		static bool collide(const Shape& a, const Shape& b, std::vector<std::pair<glm::vec3, glm::vec3>>* shapePair = nullptr);
		static Intersection::Contact intersect(const Shape& a, const Shape& b);
		//

	protected:
		//	Protected functions
		static void prepareSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction, std::vector<std::pair<glm::vec3, glm::vec3>>& simplexPoints);
		static bool containOrigin(std::vector<glm::vec3>& simplex);
		static bool isNewPoint(const glm::vec3& point, std::vector<glm::vec3>& simplex);
		//static void getIntersectStart(std::vector<std::pair<glm::vec3, glm::vec3>>& simplexPoints, glm::vec3& direction, glm::vec3* face);
		//

		//	Nested classes
		class GJKHull
		{
			public:
				//	Miscellaneous
				struct Edge;
				struct Face
				{
					Face(const glm::vec3& A1 = glm::vec3(0.f), const glm::vec3& A2 = glm::vec3(0.f), const glm::vec3& A3 = glm::vec3(0.f),
						 const glm::vec3& B1 = glm::vec3(0.f), const glm::vec3& B2 = glm::vec3(0.f), const glm::vec3& B3 = glm::vec3(0.f), const glm::vec3& normal = glm::vec3(0.f)) :
						p1(A1 - B1), p2(A2 - B2), p3(A3 - B3), a1(A1), a2(A2), a3(A3), b1(B1), b2(B2), b3(B3), n(glm::normalize(normal)), onHull(true), e1(nullptr), e2(nullptr), e3(nullptr) {};

					glm::vec3 p1, p2, p3;
					glm::vec3 a1, a2, a3;
					glm::vec3 b1, b2, b3;
					glm::vec3 n;
					bool onHull;
					Edge *e1, *e2, *e3;
				};
				struct Edge
				{
					Edge(const glm::vec3& A1 = glm::vec3(0.f), const glm::vec3& A2 = glm::vec3(0.f),
						 const glm::vec3& B1 = glm::vec3(0.f), const glm::vec3& B2 = glm::vec3(0.f)) : 
						p1(A1 - B1), p2(A2 - B2), a1(A1), a2(A2), b1(B1), b2(B2), horizonCheck(2), f1(nullptr), f2(nullptr) {};

					glm::vec3 p1, p2;
					glm::vec3 a1, a2, b1, b2;
					int horizonCheck;
					Face *f1, *f2;
				};
				struct Vertex
				{
					Vertex(const glm::vec3& P = glm::vec3(0)) : p(P) {};
					glm::vec3 p;
					bool operator< (const Vertex& rhs) const  {
						if (p.x != rhs.p.x) return p.x < rhs.p.x;
						if (p.y != rhs.p.y) return p.y < rhs.p.y;
						return p.z < rhs.p.z;
					}
				};
				//

				//	Public functions
				void initFromTetrahedron(std::vector<std::pair<glm::vec3, glm::vec3>> simplex);
				void initFromTriangle(std::vector<std::pair<glm::vec3, glm::vec3>> simplex);
				bool add(const glm::vec3& a, const glm::vec3& b);
				glm::vec3 getDirection(bool collision);
				Face* getClosestFace(bool collision);
				void draw(const glm::vec3& offset);

				void associate(Face* f, Edge* e);
				Edge* existingEdge(const glm::vec3& p1, const glm::vec3& p2);
				bool checkFaceNormal(const Face& f);

				std::list<Edge*> computeHorizon(const glm::vec3& eye);
				//

				//	Attributes
				std::set<Vertex> points;
				std::list<Edge> edges;
				std::list<Face> faces;
				//
		};
		//
};
