#include "TerrainVirtualTexture.h"
#include "TerrainArea.h"
#include "Resources/ResourceManager.h"
#include <Utiles/ProfilerConfig.h>

// Default
TerrainVirtualTexture::TerrainVirtualTexture()
{
	m_pendingTiles.reserve(1100);
}
TerrainVirtualTexture::~TerrainVirtualTexture()
{
	delete[] m_CPUTexture;
}
//

// Public methods
void TerrainVirtualTexture::initialize(int _physicalTextureSize)
{
	// CPU & GPU buffer
	m_physicalTextureSize = vec3i(_physicalTextureSize, _physicalTextureSize, 0);
	int length = m_physicalTextureSize.x * m_physicalTextureSize.y * 4;
	m_CPUTexture = new uint16_t[length];
	memset(m_CPUTexture, 0, sizeof(uint16_t) * length);

	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::MIN_NEAREST | (uint8_t)TC::MAG_NEAREST | (uint8_t)TC::WRAP_CLAMP;
	m_GPUTexture.initialize("terrainVirtualTexture", m_physicalTextureSize, m_CPUTexture, config, GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);
	ResourceManager::getInstance()->addResource(&m_GPUTexture);
	m_GPUTexture.isEnginePrivate = true;

	// Generate tiles
	const vec2i poolData[] = { vec2i(256, 2), vec2i(128, 4), vec2i(64, 5), vec2i(32, 5), vec2i(16, 5), vec2i(8, 5), vec2i(4, 4), vec2i(2, 4) };
	constexpr int poolDataSize = sizeof(poolData) / sizeof(vec2i);
	m_freeTilesPools.resize(poolDataSize);
	int startPixel = 0;
	for (int i = 0; i < poolDataSize; i++)
	{
		// pool data
		auto& pool = m_freeTilesPools[i];
		TextureTile tile;
		tile.m_owner = nullptr;
		tile.m_lod = i;
		tile.m_size = vec2i(poolData[i].x + 1);
		int columnCount = poolData[i].y;
		int rowCount = _physicalTextureSize / tile.m_size.x;
		pool.reserve(columnCount * rowCount);

		// create tiles
		for (int j = 0; j < rowCount; j++)
			for (int k = 0; k < columnCount; k++)
			{
				tile.m_min = vec2i(startPixel + k * tile.m_size.x, j * tile.m_size.y);
				tile.m_max = tile.m_min + tile.m_size;
				pool.push_back(tile);

				vec4i randColor = vec4i(rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF);
				for (int l = 0; l < tile.m_size.x; l++)
					for (int m = 0; m < tile.m_size.y; m++)
					{
						int id = (tile.m_min.x + l) * m_physicalTextureSize.y * 4 + (tile.m_min.y + m) * 4;

						m_CPUTexture[id] = randColor.x;
						m_CPUTexture[id + 1] = randColor.y;
						m_CPUTexture[id + 2] = randColor.z;
						m_CPUTexture[id + 3] = randColor.w;
					}
			}

		// increment start index
		startPixel += columnCount * tile.m_size.x;
		std::reverse(pool.begin(), pool.end());
	}

	syncroGPUTexture();
}
void TerrainVirtualTexture::updateCPUTexture(TextureTile* _destination, uint64_t* _sourceHeightMap)
{
	int spacing = TerrainArea::g_lodSpacing[_destination->m_lod];
	uint64_t* cputex64 = (uint64_t*)m_CPUTexture;
	for (int i = 0; i < _destination->m_size.x; i++)
		for (int j = 0; j < _destination->m_size.y; j++)
		{
			int destId = (_destination->m_min.x + i) * m_physicalTextureSize.y + (_destination->m_min.y + j);
			cputex64[destId] = _sourceHeightMap[i * spacing * 257 + j * spacing];
		}

	m_mutexPhysicalTexture.lock();
	m_pendingTiles.push_back(_destination);
	m_mutexPhysicalTexture.unlock();
}
void TerrainVirtualTexture::updateGPUTexture()
{
	SCOPED_CPU_MARKER("TerrainVirtualTexture::update");
	m_mutexPhysicalTexture.lock();
	if (!m_pendingTiles.empty())
	{
		vec4i lodUpdateRectSize[8];
		for (int i = 0; i < 8; i++)
			lodUpdateRectSize[i] = vec4i(m_physicalTextureSize.x, m_physicalTextureSize.y, -1, -1);

		for (auto& tile : m_pendingTiles)
		{
			if (tile->m_lod == 0)
			{
				m_GPUTexture.update(tile->m_owner->getRawData(), GL_RGBA_INTEGER, GL_UNSIGNED_SHORT,
					vec3i(tile->m_min.y, tile->m_min.x, 0),
					vec3i(tile->m_size.x, tile->m_size.y, 0));
			}
			else
			{
				vec4i& s = lodUpdateRectSize[tile->m_lod];
				s = vec4i(
					std::min(s.x, tile->m_min.x), std::min(s.y, tile->m_min.y),
					std::max(s.z, tile->m_max.x), std::max(s.w, tile->m_max.y)
				);
			}
		}

		for (int lod = 0; lod < 8; lod++)
		{
			vec4i minmax = lodUpdateRectSize[lod];
			if (minmax.x > minmax.z)
				continue;

			vec2i size = vec2i(minmax.z - minmax.x, minmax.w - minmax.y);
			uint64_t* cputex64 = (uint64_t*)m_CPUTexture;
			uint64_t* tmp = new uint64_t[size.x * size.y];
			for (int i = 0; i < size.x; i++)
				for (int j = 0; j < size.y; j++)
					tmp[i * size.y + j] = cputex64[(minmax.x + i) * m_physicalTextureSize.y + (minmax.y + j)];
			m_GPUTexture.update(tmp, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, vec3i(minmax.y, minmax.x, 0), vec3i(size.y, size.x, 0));
		}
		m_pendingTiles.clear();
	}
	m_mutexPhysicalTexture.unlock();
}
void TerrainVirtualTexture::syncroGPUTexture()
{
	m_mutexPhysicalTexture.lock();
	m_GPUTexture.update(m_CPUTexture, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);
	m_mutexPhysicalTexture.unlock();
}

TerrainVirtualTexture::TextureTile TerrainVirtualTexture::getFreeTextureTile(int _lod)
{
	m_mutexPool.lock();
	TextureTile freetile;
	freetile.m_lod = -1;
	GF_ASSERT(m_freeTilesPools[_lod].empty(), "No virtual texture tile found !");

	if (!m_freeTilesPools[_lod].empty())
	{
		freetile = m_freeTilesPools[_lod].back();
		m_freeTilesPools[_lod].pop_back();
	}
	m_mutexPool.unlock();
	return freetile;
}

void TerrainVirtualTexture::releaseTextureTile(TextureTile _textureTile)
{
	m_mutexPool.lock();
	_textureTile.m_owner = nullptr;
	m_freeTilesPools[_textureTile.m_lod].push_back(_textureTile);
	m_mutexPool.unlock();
}

GLuint TerrainVirtualTexture::getTextureId() const { return m_GPUTexture.getTextureId(); }
//
