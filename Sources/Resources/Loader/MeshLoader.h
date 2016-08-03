#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "GfMeshLoader.h"

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
