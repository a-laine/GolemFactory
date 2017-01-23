#pragma once

#include <vector>
#include <GL/glew.h>

#include "Mesh.h"
#include "Joint.h"

class MeshAnimated : public Mesh
{
    public:
		//  Default
		MeshAnimated(const std::string& meshName,                 const std::vector<glm::vec3>& verticesArray,
					 const std::vector<glm::vec3>& normalesArray, const std::vector<glm::vec3>& colorArray,
					 const std::vector<glm::ivec3>& bonesArray,   const std::vector<glm::vec3>& weightsArray,
					 const std::vector<unsigned int>& facesArray);
        ~MeshAnimated();
		//

		//	Public functions
		void initializeVBO();
		void initializeVAO();

		void draw();
        //

		//	Set / get functions
		bool isValid() const;
		//

	protected:
        //  Attributes
		GLuint weightsBuffer, bonesBuffer;

		std::vector<glm::ivec3> bones;
		std::vector<glm::vec3> weights;
        //
};
