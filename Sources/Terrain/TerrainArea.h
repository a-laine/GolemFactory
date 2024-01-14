#pragma once

#include "TerrainVirtualTexture.h"

class TerrainArea
{
	friend class Terrain;

	public:
		// Default
		TerrainArea(uint32_t seed, vec2i index);
		~TerrainArea();
		//

		// Public methods
		
		//

	protected:
		// Members
		uint32_t m_seed;
		vec2i m_areaIndex;
		vec2i m_gridIndex;
		vec4f m_cornerPosition;
		//
};