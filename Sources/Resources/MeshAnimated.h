#pragma once

#include <vector>
#include <GL/glew.h>

#include "Mesh.h"
#include "Joint.h"

class MeshAnimated : public Mesh
{
	friend class MeshSaver;

    public:
		//  Default
		MeshAnimated(const std::string& path, const std::string& meshName);
		MeshAnimated(const std::string& meshName,					const bool& isAnimable,                
					 const std::vector<glm::vec3>& verticesArray,	const std::vector<glm::vec3>& normalesArray, 
					 const std::vector<glm::vec3>& colorArray,		const std::vector<glm::ivec3>& bonesArray,   
					 const std::vector<glm::vec3>& weightsArray,	const std::vector<unsigned short>& facesArray);
        ~MeshAnimated();
		//

		//	Public functions
		void initializeVBO();
		void initializeVAO();

		void draw(const RenderOption& option = DEFAULT);
        //

		//	Set / get functions
		bool isValid() const;

		const std::vector<glm::ivec3>* getBones() const;
		const std::vector<glm::vec3>* getWeights() const;
		//

	protected:
		//	Temporary loading structures
		struct gfvertex_extended { int v, vn, c, w, b; };
		//

        //  Attributes
		GLuint weightsBuffer, bonesBuffer;
		std::vector<glm::ivec3> bones;
		std::vector<glm::vec3> weights;

		GLuint wBBoxBuffer, bBBoxBuffer;
		std::vector<glm::ivec3> bBBox;
		std::vector<glm::vec3> wBBox;
        //
};
