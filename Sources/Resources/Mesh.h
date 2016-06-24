#pragma once

#include <vector>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "ResourceVirtual.h"
#include "Loader/MeshLoader.h"

class Mesh : public ResourceVirtual
{
    public:
        //  Miscellaneous
        struct Face{
            unsigned int vertex1;
            unsigned int vertex2;
            unsigned int vertex3;
        };
        //

        //  Default
        Mesh(std::string path,std::string meshName);
        ~Mesh();

		void draw();
        bool isValid() const;
        //

        //  Set/get functions
        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;
        //

        //  Attributes
        uint8_t configuration;
		GLuint vao,vertexbuffer,arraybuffer;

        std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> color;
        std::vector<unsigned int> faces;
        //
};
