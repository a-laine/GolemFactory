#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"
#include "Loader/MeshLoader.h"

class Mesh : public ResourceVirtual
{
    public:
        //  Default
        Mesh(std::string path,std::string meshName);
		Mesh();
        ~Mesh();
		//

		//	Public functions
		void computeBoundingBoxDimension();

		void initializeVBO();
		void initializeVAO();

		void draw();
        bool isValid() const;
        //

        //  Set/get functions
        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;
        //

        //  Attributes
        uint8_t configuration;
		GLuint vao,vertexbuffer,arraybuffer,colorBuffer,normalBuffer;

        std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> color;
        std::vector<unsigned int> faces;

		glm::vec2 sizeX, sizeY, sizeZ;
        //
};
