#pragma once


#include <string>
#include <vector>
#include <GL/glew.h>
//#include <glm/glm.hpp>

#include <Terrain/Chunk.h>
#include <Resources/Shader.h>

class Map
{
	public:
		//	Default
		Map();
		~Map();
		//

		//	Public functions
		bool loadFromHeightmap(const std::string& resourceDirectory, const std::string& fileName);

		void update(const vec4f& playerPosition);
		//

		//	Set / Get
		void setShader(Shader* s);

		unsigned int getFacesCount() const;
		GLuint getVAO() const;
		Chunk* getChunk(const int& h, const  int& w);
		std::vector<vec2i> getDrawableChunks();
		mat4f getModelMatrix() const;
		vec4f getScale() const;
		Shader* getShader();

		vec2i worldToChunk(vec4f p) const;
		bool inBound(const int& x, const int& y) const;

		vec4i getExclusionZone() const;
		//

		//	Attributes
		int height, width;
		Chunk*** chunks;
		unsigned int** seeds;
		float amplitude;
		vec4f scale;
		float lodRadius[6];
		//

	private:
		//
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

		// Privates functions
		vec4f getVertex(const int& x, const int& y);
		vec4f getNormal(const int& x, const int& y);
		int getLod(float d);
		//

		//	Attributes
		GLuint  vao,
			verticesBuffer,
			normalsBuffer,
			colorsBuffer,
			facesBuffer;
		unsigned int facesCount;
		std::vector<vec2i> drawableChunks;
		vec4i exclusionZone;
		vec2i lastPlayerCell;
		std::vector<vec4i> jobList;

		Shader* shader;
		//
};