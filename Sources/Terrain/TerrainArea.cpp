#include "TerrainArea.h"

// Default
TerrainArea::TerrainArea(uint32_t seed, vec2i index) : m_seed(seed), m_areaIndex(index)
{
	m_gridIndex = vec2i(0, 0);
}

TerrainArea::~TerrainArea()
{

}
//
