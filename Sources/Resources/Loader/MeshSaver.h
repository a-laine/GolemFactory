#pragma once

#include <set>
#include <fstream>

//#include <glm/glm.hpp>

#include <Resources/Mesh.h>


class MeshSaver
{
	public:
		//	Public functions
		static void save(Mesh* mesh, const std::string& resourcesPath, std::string fileName = "", vec4f scaleModifier = vec4f(1.f));
		//

	protected:
		//	Temp structures
		struct vec4
		{
			explicit vec4(vec4f a) : x(a.x), y(a.y), z(a.z), w(a.w) {};
			bool operator<(const vec4& r) const
			{
				if (x > r.x) return false;
				else if (x == r.x && y > r.y) return false;
				else if (x == r.x && y == r.y && z > r.z) return false;
				else if (x == r.x && y == r.y && z == r.z && w > r.w) return false;
				else if (x == r.x && y == r.y && z == r.z && w == r.w) return false;
				else return true;
			};
			vec4f toVec4f() const { return vec4f(x, y, z, w); }
			float x, y, z, w;
		};
		struct ivec4
		{
			explicit ivec4(vec4i a) : x(a.x), y(a.y), z(a.z), w(a.w) {};
			bool operator<(const ivec4& r) const
			{
				if (x > r.x) return false;
				else if (x == r.x && y > r.y) return false;
				else if (x == r.x && y == r.y && z > r.z) return false;
				else if (x == r.x && y == r.y && z == r.z && w > r.w) return false;
				else if (x == r.x && y == r.y && z == r.z && w == r.w) return false;
				else return true;
			};
			vec4i toVec4i() { return vec4i(x, y, z, w); }
			int x, y, z, w;
		};

		//	Protected functions
		static void saveStatic(Mesh* mesh, std::ofstream& file, vec4f scaleModifier);
		static void saveAnimated(Mesh* mesh, std::ofstream& file, vec4f scaleModifier);

		static vec4f getTruncatedAlias(vec4f original, const float& truncature);
		//
};
