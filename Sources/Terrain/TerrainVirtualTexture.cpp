#include "TerrainVirtualTexture.h"
#include "Resources/ResourceManager.h"

// Default
TerrainVirtualTexture::TerrainVirtualTexture()
{
	m_pendingUpdateMin = vec2i(std::numeric_limits<uint16_t>::max());
	m_pendingUpdateMax = -m_pendingUpdateMin;
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
	memset(m_CPUTexture, 0, length);

	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::MIN_NEAREST | (uint8_t)TC::MAG_NEAREST | (uint8_t)TC::WRAP_CLAMP;
	m_GPUTexture.initialize("terrainVirtualTexture", m_physicalTextureSize, m_CPUTexture, config, GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);
	ResourceManager::getInstance()->addResource(&m_GPUTexture);
	m_GPUTexture.isEnginePrivate = true;

	// Generate tiles
	const vec2i poolData[] = { vec2i(256, 3), vec2i(128, 5), vec2i(64, 4), vec2i(32, 3), vec2i(16, 3), vec2i(8, 3), vec2i(4, 3), vec2i(2, 3) };
	constexpr int poolDataSize = sizeof(poolData) / sizeof(vec2i);
	m_freeTilesPools.resize(poolDataSize);
	int startPixel = 0;
	for (int i = 0; i < poolDataSize; i++)
	{
		// pool data
		auto& pool = m_freeTilesPools[i];
		TextureTile tile;
		tile.m_updateHook = nullptr;
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
	}
	updateGPUTexture();
}
void TerrainVirtualTexture::updateCPUTexture(const TextureTile& _destination, uint16_t* _source)
{
	m_mutexPhysicalTexture.lock();

	for (int l = 0; l < _destination.m_size.x; l++)
		for (int m = 0; m < _destination.m_size.y; m++)
		{
			int sourceId = l * _destination.m_size.y * 4 + m * 4;
			int destId = (_destination.m_min.x + l) * m_physicalTextureSize.y * 4 + (_destination.m_min.y + m) * 4;

			m_CPUTexture[destId] = _source[sourceId];
			m_CPUTexture[destId + 1] = _source[sourceId + 1];
			m_CPUTexture[destId + 2] = _source[sourceId + 2];
			m_CPUTexture[destId + 3] = _source[sourceId + 3];
		}

	m_pendingUpdateMin = vec2i::min(m_pendingUpdateMin, _destination.m_min);
	m_pendingUpdateMax = vec2i::max(m_pendingUpdateMax, _destination.m_max);
	m_pendingTiles.push_back((TextureTile*)&_destination);

	m_mutexPhysicalTexture.unlock();
}
void TerrainVirtualTexture::updateGPUTexture()
{
	m_mutexPhysicalTexture.lock();

	m_GPUTexture.update(m_CPUTexture, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);
	m_pendingUpdateMin = vec2i(std::numeric_limits<uint16_t>::max());
	m_pendingUpdateMax = -m_pendingUpdateMin;

	for (auto& tiles : m_pendingTiles)
	{
		if (tiles->m_updateHook)
			tiles->m_updateHook->store(true);
	}
	m_pendingTiles.clear();

	m_mutexPhysicalTexture.unlock();
}

TerrainVirtualTexture::TextureTile TerrainVirtualTexture::getFreeTextureTile(int _lod)
{
	m_mutexPool.lock();
	TextureTile freetile = m_freeTilesPools[_lod].back();
	m_freeTilesPools[_lod].pop_back();
	m_mutexPool.unlock();
	return freetile;
}

void TerrainVirtualTexture::releaseTextureTile(TextureTile& _textureTile)
{
	m_mutexPool.lock();
	_textureTile.m_updateHook = nullptr;
	m_freeTilesPools[_textureTile.m_lod].push_back(_textureTile);
	m_mutexPool.unlock();
}
//
