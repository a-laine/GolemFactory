#pragma once

#include "Shapes/Shape.h"
#include "CollisionReport.h"

#include <list>
#include <set>


class GJK
{
	public:
		//	Debug
		static bool verbose;
		static bool gizmos;
		//

		//	Public functions
		static bool collide(const Shape& a, const Shape& b, CollisionReport* report = nullptr);
		//

	protected:
		//	Usefull structs
		struct MinkowskiPoint
		{
				glm::vec4 a, b, p;

				MinkowskiPoint(const glm::vec4& _a, const  glm::vec4& _b);
				bool operator==(const MinkowskiPoint& _other) const; 
				bool operator<(const MinkowskiPoint& _other) const;
		};

		struct Edge;
		struct Face
		{
			MinkowskiPoint p1, p2, p3;
			glm::vec4 n;
			bool onHull;
			Edge *e1, *e2, *e3;

			Face(const MinkowskiPoint& _p1, const MinkowskiPoint& _p2, const MinkowskiPoint& _p3, const glm::vec4& _n);
		};

		struct Edge
		{
			MinkowskiPoint p1, p2;
			int horizonCheck;
			Face *f1, *f2;

			Edge(const MinkowskiPoint& _p1, const MinkowskiPoint& _p2);
		};

		class GJKHull
		{
			public:
				//	Public functions
				void initFromTetrahedron(const std::vector<MinkowskiPoint>& simplex);
				bool add(const MinkowskiPoint& point);
				Face* getClosestFaceToOrigin();

				void draw(const glm::vec3& offset);

				void associate(Face* f, Edge* e) const;
				Edge* existingEdge(const MinkowskiPoint& p1, const MinkowskiPoint& p2);
				bool checkFaceNormal(const Face& f) const;
				std::list<Edge*> computeHorizon(const glm::vec4& eye);
				//

				//	Attributes
				std::set<MinkowskiPoint> points;
				std::list<Edge> edges;
				std::list<Face> faces;
				//
		};
		//

		//	Protected functions
		static bool isNewPoint(const std::vector<MinkowskiPoint>& simplex, const MinkowskiPoint& p);
		static bool containOrigin(const std::vector<MinkowskiPoint>& simplex, const bool& earlyExit = true);
		static bool collide(const glm::vec4& s1a, const glm::vec4& s1b, const glm::vec4& s2a, const glm::vec4& s2b, glm::vec4& intersection);
		static bool inside(std::vector<glm::vec4>& hull, const glm::vec4& point);

		static void expandSimplex(const Shape& a, const Shape& b, CollisionReport* report, const std::vector<MinkowskiPoint>& simplex);
		static void computeManifoldContacts(const Shape& a, const Shape& b, CollisionReport* report);
		//
};
