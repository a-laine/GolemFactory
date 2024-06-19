#pragma once

#include <vector>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Physics/BoundingVolume.h"
#include "Math/TMath.h"

class Skeleton;

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
            const std::vector<vec4f>& uvArray, const std::vector<unsigned int>& facesArray,
            const std::vector<vec4i>& bonesArray, const std::vector<vec4f>& weightsArray);
        void initialize(std::vector<vec4f>* verticesArray, std::vector<vec4f>* normalsArray,
            std::vector<vec4f>* uvArray, std::vector<unsigned int>* facesArray,
            std::vector<vec4i>* bonesArray, std::vector<vec4f>* weightsArray);
		virtual void initializeVBO();
		virtual void initializeVAO();

		void setBoneNames(std::vector<std::string>& names);
		void retargetSkin(const Skeleton* skeleton);
        //

        //  Set/get functions
		bool hasSkeleton() const;

        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;

        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;
        unsigned int getNumberIndices() const;
		GLuint getIndicesType() const;
		unsigned int getFaceIndiceAt(unsigned int i) const;

		const std::vector<vec4f>* getBBoxVertices() const;
		const std::vector<unsigned short>* getBBoxFaces() const;
		const std::vector<vec4f>* getVertices() const;
		const std::vector<vec4f>* getNormals() const;
		const uint8_t* getFaces() const;
		const std::vector<vec4i>* getBones() const;
		const std::vector<vec4f>* getWeights() const;
		const AxisAlignedBox& getBoundingBox() const;

		const GLuint getVAO() const;
		const GLuint getBBoxVAO() const;

		void onDrawImGui() override;
        //


		#ifdef USE_IMGUI
			bool isEnginePrivate = false;
		#endif

	private:
        static std::string defaultName;

        void clear();
		void checkValidity();

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

		GLuint faceType;
		unsigned int faceIndicesElementSize;
		unsigned int faceIndicesCount;
		uint8_t* faceIndicesArray;

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
		std::vector<std::string> boneNames;
        //

#ifdef USE_IMGUI
		float overviewAngle0 = 0.f;
		float overviewAngle1 = 0.f;
		float overviewAngleSpeed = 0.03f;
		float overviewDistance = 1.f;
		ImVec2 clickPos;
#endif
};
