#include "TerrainArea.h"
#include "Terrain.h"

#include <Utiles/ConsoleColor.h>
#include <Utiles/ProfilerConfig.h>
#include <random>
#include <Utiles/Assert.hpp>

thread_local std::default_random_engine g_randomGenerator;

int TerrainArea::g_lodPixelCount[] = { 257, 129, 65, 33, 17, 9, 5, 3 };
int TerrainArea::g_lodSpacing[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
float TerrainArea::g_noiseCurve[] = { 0.51f, 0.0838f, 0.0459f, 0.027f, 0.013f, 0.0044f, 0.0026f, 0.0012f };
float TerrainArea::g_heightAmplitude = 300.f;	// 300m from super high to super low
float TerrainArea::g_seeLevel = 30.f;			// 30m under see is super low
int TerrainArea::g_seed = 0;

// Default
TerrainArea::TerrainArea(vec2i index, Terrain* terrain) : m_areaIndex(index), m_terrain(terrain), m_data(nullptr), m_entity(nullptr)
{
	m_gridIndex = vec2i(0, 0);
	m_tiles.resize(8);
	for (auto& tile : m_tiles)
		tile.m_lod = -1;
	m_lod = -1;
	m_hasWater = false;
}

TerrainArea::~TerrainArea()
{
	if (m_terrain && m_terrain->getVirtualTexture())
	{
		TerrainVirtualTexture* vtexture = m_terrain->getVirtualTexture();
		for (auto& tile : m_tiles)
			if (tile.m_lod >= 0)
				vtexture->releaseTextureTile(tile);
	}
	m_terrain = nullptr;
}
//

// Public methods
void TerrainArea::generate(const std::string& directory)
{
	const auto hash = [](float x, float y)
	{
		int n = x * 311 + y * 113 + g_seed;
		n = (n << 13) ^ n;
		n = n * (n * n * 15731 + 789221) + 1376312589;
		return float(n & 0x0fffffff) / float(0x0fffffff);
	};
	const auto noised = [hash](float x, float y, int octaves)
	{
		float height = 0.f;
		vec2f derivative = vec2f(0);
		float interval = 250.f;

		for (int i = 0; i < octaves; i++, interval *= 0.5f)
		{
			float amplitude = g_heightAmplitude * g_noiseCurve[i];
			float invInterval = 1.f / interval;
			vec2f p = interval * vec2f((int)((x + 0.5f) * invInterval), (int)((y + 0.5f) * invInterval));
			vec2f t = vec2f((x - p.x) * invInterval, (y - p.y) * invInterval);
			vec2f u = t * t * t * (t * (t * 6.f - vec2f(15.f)) + vec2f(10.f));
			vec2f du = 30.f * t * t * (t * (t - vec2f(2.f)) + vec2f(1.f));

			float a = amplitude * hash(p.x, p.y);
			float b = amplitude * hash(p.x + interval, p.y);
			float c = amplitude * hash(p.x, p.y + interval);
			float d = amplitude * hash(p.x + interval, p.y + interval);

			height += a + (b - a) * u.x + (c - a) * u.y + (a - b - c + d) * u.x * u.y;
			derivative += (du * (vec2f(b - a, c - a) + (a - b - c + d) * vec2f(u.y, u.x))) / amplitude;
		}
		return vec3f(height - g_seeLevel, derivative.x, derivative.y);
	};
	const auto materialSelect = [&](const MapData& data, int i, int j)
	{
		float height = data.height + 3.0 * hash(i, j);
		if (std::min(height, data.height) < 0.1)
			return (hash(j, i) > 0.2f ? 4 : 5);    // sand
		else if (std::min(height, data.height) < 0.7)
			return 7;    // dirt
		else if (height > 0.5 * g_heightAmplitude)
			return 8;	  // snow
		return 0;
	};

	// direct write noised and derivative
	constexpr float faceScale = 250.f / 256.f;
	MapData** data = new MapData*[257];
	for (int i = 0; i < 257; i++)
		data[i] = new MapData[257];
	for (int i = 0; i < 257; i++)
		for (int j = 0; j < 257; j++)
		{
			vec3f n = noised(m_gridIndex.x * 250 + faceScale * i, m_gridIndex.y * 250 + faceScale * j, 6);
			data[i][j].height = clamp(n.x, -g_seeLevel, g_heightAmplitude - g_seeLevel);
			data[i][j].water = 0.f;
			data[i][j].hole = 0;
			data[i][j].normal = vec4f(n.y, 4, n.z, 0).getNormal();
			data[i][j].material = materialSelect(data[i][j], 256 * m_gridIndex.x + i, 256 * m_gridIndex.y + j);
		}

	// write to texture
	std::vector<uint64_t> texture(257 * 257);
	for (int i = 0; i < 257; i++)
		for (int j = 0; j < 257; j++)
			texture[i * 257 + j] = data[i][j].pack();

	// save to file
	std::string filename = getNameFromIndex() + ".area";
	std::ofstream file(directory + filename, std::ios::out | std::ios::binary);
	if (file)
	{
		file.write((const char*)texture.data(), 257 * 257 * sizeof(uint64_t));
		file.close();
	}
	else
	{
		std::cout << ConsoleColor::red << "[ERROR] : cannot create file " << filename << ConsoleColor::classic << std::endl;
	}

#if 0
	// send to virtual texture
	if (m_terrain && m_terrain->getVirtualTexture())
	{
		TerrainVirtualTexture* vtexture = m_terrain->getVirtualTexture();
		for (int lod = 0; lod < 8; lod++)
		{
			m_tiles[lod] = vtexture->getFreeTextureTile(lod);
			if (m_tiles[lod].m_lod >= 0)
			{
				m_tiles[lod].m_owner = this;
				vtexture->updateCPUTexture(&m_tiles[lod], texture.data());
			}
		}
	}
#endif

	// clear all
	for (int i = 0; i < 257; i++)
		delete[] data[i];
	delete[] data;
}
std::string TerrainArea::getNameFromIndex() const
{
	return (m_areaIndex.x < 0 ? "n" : "") + std::to_string(std::abs(m_areaIndex.x)) + "_" +
		   (m_areaIndex.y < 0 ? "n" : "") + std::to_string(std::abs(m_areaIndex.y));
}
vec4f TerrainArea::getCenter() const
{
	return m_center;
}
const uint64_t* TerrainArea::getRawData() const
{
	return m_data;
}
void TerrainArea::setLod(int lod)
{
	// unload heightmap
	if (m_lod >= 0 && lod < 0)
	{
		delete[] m_data;
		m_data = nullptr;
	}

	// load heightmap
	else if (m_lod < 0 && lod >= 0)
	{
		m_data = new uint64_t[257 * 257];
		std::string filename = getNameFromIndex() + ".area";
		std::ifstream file(m_terrain->getDirectory() + "/" + filename, std::ios::binary);
		GF_ASSERT(file, "No heightmap file found !");

		if (file)
		{
			file.read((char*)m_data, 257 * 257 * sizeof(uint64_t));
			file.close();
		}

		constexpr float maxValue = std::numeric_limits<float>::max();
		float minHeight = maxValue;
		float maxHeight = -maxValue;
		MapData data;
		//m_hasWater = true;
		for (int i = 0; i < 257 * 257; i++)
		{
			data.unpack(m_data[i]);
			if (data.water > data.height)
				m_hasWater = true;

			minHeight = std::min(minHeight, data.water);
			minHeight = std::min(minHeight, data.height);
			maxHeight = std::max(maxHeight, data.water);
			maxHeight = std::max(maxHeight, data.height);
		}

		m_boundingBox.min = m_center + vec4f(-125.f, minHeight, -125.f, 0.f);
		m_boundingBox.max = m_center + vec4f( 125.f, maxHeight,  125.f, 0.f);
	}

	TerrainVirtualTexture* vtexture = m_terrain->getVirtualTexture();
	if (vtexture)
	{
		int tileLod = lod == -1 ? 100 : lod;
		for (int l = 0; l < 8; l++)
		{
			// release texture tile
			if (l < tileLod)
			{
				if (m_tiles[l].m_lod >= 0)
				{
					vtexture->releaseTextureTile(m_tiles[l]);
					m_tiles[l].m_lod = -1;
				}
			}

			// aquire texture tile
			else
			{
				if (m_tiles[l].m_lod < 0)
				{
					m_tiles[l] = vtexture->getFreeTextureTile(l);
					GF_ASSERT(m_tiles[l].m_lod >= 0, "No virtual texture tile found !");

					if (m_tiles[l].m_lod >= 0)
					{
						m_tiles[l].m_owner = this;
						vtexture->updateCPUTexture(&m_tiles[l], m_data);
					}
				}
			}
		}
	}
	m_lod = lod;
}
int TerrainArea::getLod()
{
	return m_lod;
}
Terrain* TerrainArea::getTerrain() const
{
	return m_terrain;
}
bool TerrainArea::hasWater() const
{
	return m_hasWater;
}
AxisAlignedBox TerrainArea::getBoundingBox() const
{
	return m_boundingBox;
}
//

uint64_t TerrainArea::MapData::pack()
{
	float scaleFactor = 65535.f / TerrainArea::g_heightAmplitude;
	constexpr float normalFactor = 1024.f;

	uint64_t h = (uint64_t)((uint16_t)((height + g_seeLevel) * scaleFactor));
	uint64_t w = (uint64_t)((uint16_t)((water + g_seeLevel) * scaleFactor));
	uint64_t nx = (uint64_t)((uint16_t)(clamp((normal.x + 1.f) * normalFactor, 0.f, 2047.f)));
	uint64_t nz = (uint64_t)((uint16_t)(clamp((normal.z + 1.f) * normalFactor, 0.f, 2047.f)));
	uint64_t m = (uint64_t)material;
	uint64_t hh = (uint64_t)(hole & 0x03);

	return h | (w<<16) | (nx<<32) | (nz<<43) | (m<<54) | (hh<<62);
}
void TerrainArea::MapData::unpack(uint64_t data)
{
	float scaleFactor = TerrainArea::g_heightAmplitude / 65535.f;
	constexpr float normalFactor = 1.f / 1024.f;

	height = (data & 0xFFFF) * scaleFactor - g_seeLevel;
	water = ((data >> 16) & 0xFFFF) * scaleFactor - g_seeLevel;
	float nx = ((data >> 32) & 0x7FF) * normalFactor - 1.f;
	float nz = ((data >> 43) & 0x7FF) * normalFactor - 1.f;
	normal = vec4f(nx, std::sqrt(1.f - std::min(nx * nx + nz * nz, 1.f)), nz, 0.f);
	material = (data >> 54) & 0xFF;
	hole = data >> 62;
}