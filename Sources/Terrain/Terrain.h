#pragma once

#include "TerrainArea.h"

#include <map>
#include <vector>

class Terrain
{
	public:
		// Default
		Terrain();
		~Terrain();
		//

		// public functions
		void initializeClipmaps();

		void AddArea(vec2i index, TerrainArea& area, bool recomputeIfNeeded = true);
		void RemoveArea(vec2i index);

		void RecomputeGrid();
		//


	protected:
		// structs
		struct vec2i_ordered
		{
			vec2i_ordered(int x, int y) : v(x, y) {};
			vec2i_ordered(const vec2f& u) : v(u) {};

			vec2i v;
			bool operator<(const vec2i_ordered& o) const
			{
				if (v.x != o.v.x)
					return v.x < o.v.x;
				else return v.y < o.v.y;
			}
		};
		struct Clipmap
		{
			GLuint m_VAO, m_vertexbuffer, m_arraybuffer;
			std::vector<vec4f> m_vertices;
			std::vector<uint16_t> m_indices;
		};
		//
		
		// Members
		std::map<vec2i_ordered, TerrainArea> m_areas;
		vec2i m_gridSize, m_gridMinIndex;
		TerrainArea*** m_grid;

		std::vector<Clipmap> m_clipmaps;
		//
};