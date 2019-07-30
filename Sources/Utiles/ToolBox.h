#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Utiles/Parser/Variant.h"

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
		static Variant getFromVec2(const glm::vec2& vec);
		static Variant getFromVec3(const glm::vec3& vec);
		static Variant getFromVec4(const glm::vec4& vec);
		static Variant getFromQuat(const glm::fquat& quat);
		static Variant getFromMat4(const glm::mat4& mat);

		//	mesh cleaner
		static void optimizeStaticMesh( std::vector<glm::vec3>& verticesArray,
									    std::vector<glm::vec3>& normalesArray,
										std::vector<glm::vec3>& colorArray,
										std::vector<unsigned short>&facesArray );

		//	hull mesh optimazer
		static void optimizeHullMesh(Mesh* mesh);
};
