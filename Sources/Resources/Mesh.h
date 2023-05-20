#pragma once

#include <vector>
#include <GL/glew.h>
//#include <glm/glm.hpp>

#include "ResourceVirtual.h"
#include "Physics/BoundingVolume.h"
#include "Math/TMath.h"


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

        void initialize(const std::vector<vec4f>& verticesArray, const std::vector<vec4f>& normalsArray,
            const std::vector<vec4f>& uvArray, const std::vector<unsigned short>& facesArray,
            const std::vector<vec4i>& bonesArray, const std::vector<vec4f>& weightsArray);
        void initialize(std::vector<vec4f>* verticesArray, std::vector<vec4f>* normalsArray,
            std::vector<vec4f>* uvArray, std::vector<unsigned short>* facesArray,
            std::vector<vec4i>* bonesArray, std::vector<vec4f>* weightsArray);
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

		const std::vector<vec4f>* getBBoxVertices() const;
		const std::vector<unsigned short>* getBBoxFaces() const;
		const std::vector<vec4f>* getVertices() const;
		const std::vector<vec4f>* getNormals() const;
		const std::vector<vec4f>* getUVs() const;
		const std::vector<unsigned short>* getFaces() const;
		const std::vector<vec4i>* getBones() const;
		const std::vector<vec4f>* getWeights() const;
		const AxisAlignedBox& getBoundingBox() const;

		const GLuint getVAO() const;
		const GLuint getBBoxVAO() const;

		void onDrawImGui() override;
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
				uvsBuffer,
				normalsBuffer,
				facesBuffer;

        std::vector<vec4f> vertices;
		std::vector<vec4f> normals;
		std::vector<vec4f> uvs;
        std::vector<unsigned short> faces;

		GLuint  BBoxVao, vBBoxBuffer, fBBoxBuffer;
		std::vector<vec4f> vBBox;
		std::vector<unsigned short> fBBox;

        // Animated
        GLuint weightsBuffer, bonesBuffer;
        std::vector<vec4i> bones;
        std::vector<vec4f> weights;

        GLuint wBBoxBuffer, bBBoxBuffer;
        std::vector<vec4i> bBBox;
        std::vector<vec4f> wBBox;
        //
};
