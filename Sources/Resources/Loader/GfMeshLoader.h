#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <iterator>
#include <algorithm>
#include <glm/glm.hpp>

class GfMeshLoader
{
	public:
		//  Public functions
		int load(std::string file);
		int pack(std::vector<glm::vec3>* vertices,
				 std::vector<glm::vec3>* normales,
				 std::vector<glm::vec3>* color,
				 std::vector<glm::vec2>* texture,
				 std::vector<glm::vec2>* weight,
				 std::vector<unsigned int>* faces);
		//


	protected:
		//	Miscellaneous
		enum flags {
			none = 0,
			hasVertices = (1 << 0),
			hasNormals  = (1 << 1),
			hasColors   = (1 << 2),
			hasTextures = (1 << 3),
			hasWeights  = (1 << 4)
		};
		//

		//  Attributes
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> color;
		std::vector<glm::vec2> texture;
		std::vector<glm::vec3> weight;
		std::vector<unsigned int> faces;
		//

	private:
		//	Private functions
		void removeWhiteSpace(std::string& input);
		//
};

