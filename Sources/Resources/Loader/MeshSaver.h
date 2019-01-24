#pragma once

#include <set>
#include <fstream>

#include <glm/glm.hpp>

#include <Resources/Mesh.h>


class MeshSaver
{
	public:
		//	Public functions
		static void save(Mesh* mesh, const std::string& resourcesPath, std::string fileName = "", glm::vec3 scaleModifier = glm::vec3(1.f));
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
			glm::vec3 toGlm() const { return glm::vec3(x, y, z); }
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
			glm::ivec3 toGlm() { return glm::ivec3(x, y, z); }
			int x, y, z;
		};

		struct vertex { int position, normal, color; };
		struct face { vertex vertex1, vertex2, vertex3; };
		
		struct extendedVertex { int position, normal, color, weights, bones; };
		struct extendedFace { extendedVertex vertex1, vertex2, vertex3; };
		//

		//	Protected functions
		static void saveStatic(Mesh* mesh, std::ofstream& file, glm::vec3 scaleModifier);
		static void saveAnimated(Mesh* mesh, std::ofstream& file, glm::vec3 scaleModifier);

		static glm::vec3 getTruncatedAlias(glm::vec3 original, const float& truncature);
		//
};
