#pragma once


#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Terrain/Chunk.h"
#include "Resources/Shader.h"

class Map
{
	public:
		//	Default
		Map();
		~Map();
		//

		//	Public functions
		bool loadFromHeightmap(const std::string& resourceDirectory, const std::string& fileName);

		void update(const glm::vec3& playerPosition);
		//

		//	Set / Get
		void setShader(Shader* s);

		unsigned int getFacesCount() const;
		GLuint getVAO() const;
		Chunk* getChunk(const int& h, const  int& w);
		std::vector<glm::ivec2> getDrawableChunks();
		glm::mat4 getModelMatrix() const;
		glm::vec3 getScale() const;
		Shader* getShader();

		glm::ivec2 worldToChunk(glm::vec3 p) const;
		bool inBound(const int& x, const int& y) const;

		glm::ivec4 getExclusionZone() const;
		//

		//	Attributes
		int height, width;
		Chunk*** chunks;
		unsigned int** seeds;
		float amplitude;
		glm::vec3 scale;
		//

	private:
		//
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

		// Privates functions
		glm::vec3 getVertex(const int& x, const int& y);
		glm::vec3 getNormal(const int& x, const int& y);
		int getLod(float d);
		//

		//	Attributes
		GLuint  vao,
			verticesBuffer,
			normalsBuffer,
			colorsBuffer,
			facesBuffer;
		unsigned int facesCount;
		std::vector<glm::ivec2> drawableChunks;
		glm::ivec4 exclusionZone;
		glm::ivec2 lastPlayerCell;
		std::vector<glm::ivec4> jobList;

		Shader* shader;
		//
};