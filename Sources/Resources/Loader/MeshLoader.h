#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

class MeshLoader
{
    public:
        //  Public functions
        static int loadMesh(std::string file,
							std::vector<glm::vec3>& vertices,
							std::vector<glm::vec3>& normales,
							std::vector<glm::vec3>& color,
							std::vector<unsigned int>& faces,
							bool& hasSkeleton);
        //

	private:
		//	Private functions
		static int loadGfMesh(std::string file,
							  std::vector<glm::vec3>& vertices,
							  std::vector<glm::vec3>& normales,
							  std::vector<glm::vec3>& color,
							  std::vector<unsigned int>& faces,
							  bool& hasSkeleton);
		//
};
