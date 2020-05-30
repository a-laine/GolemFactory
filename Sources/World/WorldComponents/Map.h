#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Map
{
	public:
		//	Default
		Map();
		~Map();
		//

		//	Public functions
		bool loadFromHeightmap(const std::string& resourceDirectory, const std::string& fileName);
		//

		//	Set / Get
		unsigned int getFacesCount() const;
		GLuint getVAO() const;
		//

		//	Attributes
		int height, width;
		float** heightmap;
		float amplitude, cellSize;
		//

	private:
		//	Attributes
		GLuint  vao,
			verticesBuffer,
			normalsBuffer,
			colorsBuffer,
			facesBuffer;
		unsigned int facesCount;
		//
};