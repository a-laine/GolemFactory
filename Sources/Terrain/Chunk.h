#pragma once

#include <vector>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>


class Chunk
{
	friend class Map;

	public:
		//	Default
		Chunk(unsigned int seed, glm::mat4 model, const float& cornerHeight);
		~Chunk();
		//

		//	Public functions
		void initialize(const float& topLeft, const float& topRight, const float& bottomRight);
		void free();
		void addLOD();
		void removeLOD();

		void initializeVBO();
		void initializeVAO();
		void updateVBO();
		//

		//	Set / Get
		bool isInitialized() const;

		void setModelMatrix(glm::mat4 model);

		glm::mat4 getModelMatrix() const;
		float* getModelMatrixPtr();
		uint8_t getLod() const;
		float getCorner();
		std::vector<float> getLeftBorder(const uint8_t& targetLod);
		std::vector<float> getBottomBorder(const uint8_t& targetLod);
		bool getNeedVBOUpdate() const;
		unsigned int getFacesCount() const;
		GLuint getVAO() const;
		//

		//	Shared attributes
		static uint8_t maxSubdivide;
		static float rugosity;
		//

	private:
		//	Private functions
		void splitFace(const unsigned int& i0, const unsigned int& i1, const unsigned int& i2, const unsigned int& i3, const float& amplitude);
		void mergeFace(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3);
		
		float randf(const float& min = -1.f, const float& max = 1.f, const unsigned int& quantum = 32768) const;
		unsigned int randi();
		void randJump(const unsigned int& jumpCount);
		//

		//	Miscellaneous
		struct gfvertex
		{  
			gfvertex(const float& x, const float& y) : v(x, y) {};
			gfvertex(const glm::vec3& u) : v(u.x, u.y) {};
			gfvertex(const glm::vec2& u) : v(u) {};

			glm::vec2 v;
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
		glm::mat4 modelMatrix;
		float corner;

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<unsigned int> faces;

		std::map<gfvertex, unsigned int> indexes;
		std::vector<unsigned int> lodVerticesCount;

		GLuint  vao,
			verticesBuffer,
			normalsBuffer,
			colorsBuffer,
			facesBuffer;
		//
};