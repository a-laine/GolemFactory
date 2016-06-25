#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <limits>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class MeshLoader
{
    public:
        //  Public functions
        static int loadMesh(std::string file,
							std::vector<glm::vec3>& vertices,
							std::vector<glm::vec3>& normales,
							std::vector<glm::vec3>& color,
							std::vector<unsigned int>& faces);
        //
};
