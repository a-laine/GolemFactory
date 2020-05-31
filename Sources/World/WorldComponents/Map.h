#pragma once


#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Terrain/Chunk.h"

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
		unsigned int getFacesCount() const;
		GLuint getVAO() const;
		Chunk* getChunk(const int& h, const  int& w);
		std::vector<glm::ivec2> getDrawableChunks();
		glm::mat4 getModelMatrix() const;

		glm::ivec2 worldToChunk(glm::vec3 p) const;
		bool inBound(const int& x, const int& y) const;
		//

		//	Attributes
		int height, width;
		Chunk*** chunks;
		unsigned int** seeds;
		float amplitude;
		float scale;
		//

	private:
		//	Attributes
		GLuint  vao,
			verticesBuffer,
			normalsBuffer,
			colorsBuffer,
			facesBuffer;
		unsigned int facesCount;
		std::vector<glm::ivec2> drawableChunks;
		glm::ivec2 lastPlayerPos;
		//
};