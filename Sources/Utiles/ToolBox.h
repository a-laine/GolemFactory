#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

/*#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>*/
#include "Math/TMath.h"

#include "Parser/Variant.h"

class Mesh;

class ToolBox
{
	public:
		//	file and string stuff
		static std::string openAndCleanCStyleFile(std::string targetFile, std::string commentBlockEntry = "/*", std::string commentLineEntry = "//");
		static void clearWhitespace(std::string& input);
		static bool isPathExist(const std::string& fileName);

		template <typename T> static std::string to_string_with_precision(const T& a_value, const int& n = 2)
		{
			std::ostringstream out;
			out << std::setprecision(n) << a_value;
			return out.str();
		}

		//	vector and matrix serialization
		static Variant getFromVec2f(const vec2f& vec);
		static Variant getFromVec3f(const vec3f& vec);
		static Variant getFromVec4f(const vec4f& vec);
		static Variant getFromQuatf(const quatf& quat);
		static Variant getFromMat4f(const mat4f& mat);

		//	mesh cleaner
		static void optimizeStaticMesh( std::vector<vec4f>& verticesArray,
									    std::vector<vec4f>& normalesArray,
										std::vector<vec4f>& colorArray,
										std::vector<unsigned short>&facesArray );

		//	hull mesh optimazer
		static void optimizeHullMesh(Mesh* mesh);
};
