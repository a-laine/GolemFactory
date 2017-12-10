#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"

class Mesh : public ResourceVirtual
{
	friend class HouseGenerator;
	friend class MeshSaver;

    public:
		//  Miscellaneous
		enum ConfigurationFlags
		{
			WELL_LOADED = 1 << 0,
			HAS_SKELETON = 1 << 1,
			IS_ANIMABLE = 1 << 2
		};
		//

        //  Default
		Mesh(const std::string& meshName = "unknown.unknown");
		Mesh(const std::string& path, const std::string& meshName);
		Mesh(const std::string& meshName, const std::vector<glm::vec3>& verticesArray, 
			const std::vector<glm::vec3>& normalesArray, const std::vector<glm::vec3>& colorArray, 
			const std::vector<unsigned short>& facesArray);
		Mesh(const Mesh& original);
		virtual ~Mesh();
		//

		//	Public functions
		void computeBoundingBox();

		virtual void initializeVBO();
		virtual void initializeVAO();

		virtual void draw();
		virtual void drawBB();
        //

        //  Set/get functions
		bool hasSkeleton() const;
		bool isAnimable() const;
		virtual bool isValid() const;
		bool isFromGolemFactoryFormat() const;

        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;

		const std::vector<glm::vec3>* getBBoxVertices() const;
		const std::vector<unsigned short>* getBBoxFaces() const;
		const std::vector<glm::vec3>* getVertices() const;
		const std::vector<unsigned short>* getFaces() const;
		virtual const std::vector<glm::ivec3>* getBones() const;
		virtual const std::vector<glm::vec3>* getWeights() const;
        //

		//	Attributes
		static std::string extension;
		glm::vec3 aabb_min, aabb_max;
		//

	protected:
		//	Temporary loading structures
		struct gfvertex { int v, vn, c; };
		//

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
        std::vector<unsigned short> faces;

		GLuint  BBoxVao, vBBoxBuffer, fBBoxBuffer;
		std::vector<glm::vec3> vBBox;
		std::vector<unsigned short> fBBox;
        //
};
