#pragma once

#include <set>
#include <fstream>

#include <glm/glm.hpp>

#include "../Mesh.h"
#include "../MeshAnimated.h"


class MeshSaver
{
	public:
		//  Default
		MeshSaver();
		~MeshSaver();
		//

		//	Public functions
		void save(Mesh* mesh, const std::string& resourcesPath, std::string fileName = "");
		//

	protected:
		//	Temp structures
		struct vec3
		{
			vec3(glm::vec3 a) : x(a.x), y(a.y), z(a.z) {};
			bool operator<(const vec3& r) const
			{
				if (x > r.x) return false;
				else if (x == r.x && y > r.y) return false;
				else if (x == r.x && y == r.y && z > r.z) return false;
				else if (x == r.x && y == r.y && z == r.z) return false;
				else return true;
			};
			float x, y, z;
		};
		struct ivec3
		{
			ivec3(glm::ivec3 a) : x(a.x), y(a.y), z(a.z) {};
			bool operator<(const ivec3& r) const
			{
				if (x > r.x) return false;
				else if (x == r.x && y > r.y) return false;
				else if (x == r.x && y == r.y && z > r.z) return false;
				else if (x == r.x && y == r.y && z == r.z) return false;
				else return true;
			};
			int x, y, z;
		};

		struct vertex { int position, normal, color; };
		struct face { vertex vertex1, vertex2, vertex3; };
		
		struct extendedVertex { int position, normal, color, weights, bones; };
		struct extendedFace { extendedVertex vertex1, vertex2, vertex3; };
		//

		//	Protected functions
		void clear();

		void save(Mesh* mesh, std::ofstream& file);
		void save(MeshAnimated* mesh, std::ofstream& file);
		//

		//	Attributes
		std::set<vec3> vertices;
		std::set<vec3> normales;
		std::set<vec3> colors;
		std::set<vec3> weights;
		std::set<ivec3> bones;
		std::vector<unsigned int> faces;
		//
};
