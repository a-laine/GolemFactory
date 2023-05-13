#pragma once

#include <Resources/Mesh.h>

#include <vector>
#include <set>

class GiftWrapping
{
	public:
		GiftWrapping();
		Mesh* getConvexHull(Mesh* m);


	
		struct vec3
		{
			explicit vec3(vec4f v) :x(v.x), y(v.y), z(v.z) {}
			friend bool operator<(const vec3& l, const vec3& r)
			{
				if (l.x != r.x) return l.x < r.x;
				else if (l.y != r.y) return l.y < r.y;
				else return l.z < r.z;
			}
			vec4f vec() const { return vec4f(x, y, z, 1); }
			float x, y, z;
		};
		struct Face
		{
			Face() : p1(0.f), p2(0.f), p3(0.f), n(0.f) {};
			Face(const vec4f& point1, const vec4f& point2, const vec4f& point3, const vec4f& normal) : p1(point1), p2(point2), p3(point3), n(normal) {};

			friend bool operator<(const Face& l, const Face& r)
			{
				if (l.p1 != r.p1) return vec3(l.p1) < vec3(r.p1);
				else if (l.p2 != r.p2) return vec3(l.p2) < vec3(r.p2);
				else return vec3(l.p3) < vec3(r.p3);
			}

			vec4f n;
			vec4f p1, p2, p3;
		};
		struct Edge
		{
			Edge() : p1(0.f), p2(0.f), firstFaceP3(0.f) {};
			Edge(const vec4f& point1, const vec4f& point2, const vec4f& point3) : firstFaceP3(point3)
			{
				if (vec3(point1) < vec3(point2))
				{
					p1 = point1;
					p2 = point2;
				}
				else
				{
					p1 = point2;
					p2 = point1;
				}
			};

			friend bool operator<(const Edge& l, const Edge& r)
			{
				if (l.p1 != r.p1) return vec3(l.p1) < vec3(r.p1);
				else return vec3(l.p2) < vec3(r.p2);
			}

			vec4f p1, p2;
			vec4f firstFaceP3;
		};

	protected:
		void initializeHull(const std::vector<vec4f>& pointCloud);

		bool degenerated;
		std::set<vec3> cloud;
		std::vector<Edge> edgeStack;
		std::set<Edge> hullEdges;
		std::set<Face> hullFaces;
};