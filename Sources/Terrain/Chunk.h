#pragma once

#include <vector>
#include <map>
#include <GL/glew.h>
//#include <glm/glm.hpp>
#include "Math/TMath.h"

class Chunk
{
	public:
		//	Default
		Chunk(unsigned int seed, mat4f model, const float& cornerHeight);
		~Chunk();
		//

		//	Public functions
		void initialize(const float& topLeft, const float& topRight, const float& bottomRight);
		void free();
		void addLOD(const unsigned int& seed1, const unsigned int&  seed2, bool debug);
		void removeLOD();

		void initializeVBO();
		void initializeVAO();
		void updateVBO();
		//

		//	Set / Get
		bool isInitialized() const;

		void setModelMatrix(mat4f model);

		mat4f getModelMatrix() const;
		float* getModelMatrixPtr();
		uint8_t getLod() const;
		float getCorner();
		unsigned int getSeed();
		bool getNeedVBOUpdate() const;
		unsigned int getCenterFacesCount() const;
		unsigned int getBorderFacesCount() const;
		unsigned int getSeamlessBorderFacesCount() const;
		GLuint getVAO() const;
		//

		//	Shared attributes
		static uint8_t maxSubdivide;
		static float rugosity;
		//

	private:
		//	Private functions
		void splitFace(const unsigned int& i0, const unsigned int& i1, const unsigned int& i2, const unsigned int& i3, const float& amplitude, const float& step, const uint8_t& faceMode);
		void mergeFace(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3);
		
		float randf(const float& min = -1.f, const float& max = 1.f, const unsigned int& quantum = 32768);
		unsigned int randi();
		void initRandomNumberGenerator(const unsigned int& offset, const unsigned int& newseed);

		unsigned int instantiateVertex(const vec4f& v, const float&  amplitude);
		unsigned int instantiateVertex(const float& x, const float& y, const float& z, const float&  amplitude);
		unsigned int instantiateVertexSmooth(const vec4f& v, const float& step, const float&  amplitude);

		void smoothNormals(const float& step);
		void splitCenter(const float& step, const float& amplitude);
		void instantiateBorders(const unsigned int&  seed1, const unsigned int&  seed2, const int& offset, const int subdivision, const float& step, const float& amplitude);
		void generateBorders(const float& step);
		void generateSeamlessBorders(const float& step);
		void generateFaces(const float& x, const float& y, const float& step, const uint16_t& faceMode);
		//

		//	Miscellaneous
		struct gfvertex
		{  
			gfvertex(const float& x, const float& y) : v(x, y) {};
			gfvertex(const vec4f& u) : v(u.x, u.y) {};
			gfvertex(const vec2f& u) : v(u) {};

			vec2f v;
			bool operator<(const gfvertex& o) const
			{
				if (v.x != o.v.x)
					return v.x < o.v.x;
				else return v.y < o.v.y;
			}
		};
		//


		//	Attributes
		bool needVBOUpdate;
		bool initialized;
		uint8_t lod;
		unsigned int seed;
		unsigned long int next;
		mat4f modelMatrix;
		float corner;

		std::vector<vec4f> vertices;
		std::vector<vec4f> normals;
		std::vector<vec4f> colors;
		std::vector<unsigned int> faces;
		unsigned int facesCenter;
		unsigned int facesBorder;
		unsigned int facesBorderSeamless;

		std::map<gfvertex, unsigned int> indexes;
		std::vector<unsigned int> lodVerticesCount;

		GLuint  vao,
			verticesBuffer,
			normalsBuffer,
			colorsBuffer,
			facesBuffer;
		//
};