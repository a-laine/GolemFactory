#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"

class Mesh : public ResourceVirtual
{
	friend class HouseGenerator;

    public:
		//  Miscellaneous
		enum ConfigurationFlags
		{
			VALID = 1 << 0,
			HAS_SKELETON = 1 << 1
		};
		//

        //  Default
		Mesh(const std::string& meshName);
		Mesh(const std::string& meshName, const std::vector<glm::vec3>& verticesArray, const std::vector<glm::vec3>& normalesArray, const std::vector<glm::vec3>& colorArray, const std::vector<unsigned int>& facesArray);
        virtual ~Mesh();
		//

		//	Public functions
		void computeBoundingBoxDimension();

		virtual void initializeVBO();
		virtual void initializeVAO();

		virtual void draw();
        //

        //  Set/get functions
        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;
		virtual bool isValid() const;
		bool hasSkeleton() const;
        //

		//	Attributes
		glm::vec2 sizeX, sizeY, sizeZ;
		//

	protected:
        //  Attributes
        uint8_t configuration;
		GLuint  vao,
				verticesBuffer,
				colorsBuffer,
				normalsBuffer,
				facesBuffer;

        std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> colors;
        std::vector<unsigned int> faces;
        //
};
