#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"
#include "Physics/BoundingVolume.h"


class Mesh : public ResourceVirtual
{
	friend class ToolBox;
	friend class HouseGenerator;
	friend class MeshSaver;
	friend class IncrementalHull;

    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);


        //  Default
		Mesh(const std::string& meshName = "unknown");
		virtual ~Mesh();
		//

		//	Public functions
		void computeBoundingBox();

        void initialize(const std::vector<glm::vec3>& verticesArray, const std::vector<glm::vec3>& normalsArray,
            const std::vector<glm::vec3>& colorArray, const std::vector<unsigned short>& facesArray,
            const std::vector<glm::ivec3>& bonesArray, const std::vector<glm::vec3>& weightsArray);
        void initialize(std::vector<glm::vec3>&& verticesArray, std::vector<glm::vec3>&& normalsArray,
            std::vector<glm::vec3>&& colorArray, std::vector<unsigned short>&& facesArray,
            std::vector<glm::ivec3>&& bonesArray, std::vector<glm::vec3>&& weightsArray);
		virtual void initializeVBO();
		virtual void initializeVAO();

		/*virtual void draw();
		virtual void drawGroup(unsigned short groupCount);
		virtual void drawBB();*/
        //

        //  Set/get functions
		bool hasSkeleton() const;

        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;

        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;

		const std::vector<glm::vec3>* getBBoxVertices() const;
		const std::vector<unsigned short>* getBBoxFaces() const;
		const std::vector<glm::vec3>* getVertices() const;
		const std::vector<glm::vec3>* getNormals() const;
		const std::vector<unsigned short>* getFaces() const;
		const std::vector<glm::ivec3>* getBones() const;
		const std::vector<glm::vec3>* getWeights() const;
		const AxisAlignedBox& getBoundingBox() const;

		const GLuint getVAO() const;
		const GLuint getBBoxVAO() const;
        //

	private:
        static std::string defaultName;

        void clear();


		//	Temporary loading structures
		struct gfvertex { int v, vn, c; };
        struct gfvertex_extended { int v, vn, c, w, b; };
		//

        //  Attributes
		AxisAlignedBox boundingBox;

		GLuint  vao,
				verticesBuffer,
				colorsBuffer,
				normalsBuffer,
				facesBuffer;

        std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
        std::vector<unsigned short> faces;

		GLuint  BBoxVao, vBBoxBuffer, fBBoxBuffer;
		std::vector<glm::vec3> vBBox;
		std::vector<unsigned short> fBBox;

        // Animated
        GLuint weightsBuffer, bonesBuffer;
        std::vector<glm::ivec3> bones;
        std::vector<glm::vec3> weights;

        GLuint wBBoxBuffer, bBBoxBuffer;
        std::vector<glm::ivec3> bBBox;
        std::vector<glm::vec3> wBBox;
        //
};
