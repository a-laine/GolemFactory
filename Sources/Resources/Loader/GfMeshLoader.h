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
		enum flagsRanks {
			rankCount = 5,
			rankVertices = 0,
			rankNormals  = 1,
			rankColors   = 2,
			rankTextures = 3,
			rankWeights  = 4
		};
		enum flags {
			none = 0,
			hasVertices = (1 << rankVertices),
			hasNormals  = (1 << rankNormals),
			hasColors   = (1 << rankColors),
			hasTextures = (1 << rankTextures),
			hasWeights  = (1 << rankWeights)
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
		void removeWhiteSpaceAndComment(std::string& input);
		//
};

