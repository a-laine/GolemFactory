#include "TerrainArea.h"
#include "Terrain.h"
#include "TerrainDetailDrawableComponent.h"

#include <Utiles/ConsoleColor.h>
#include <Utiles/ProfilerConfig.h>
#include <random>
#include <Utiles/Assert.hpp>
#include <Resources/ResourceManager.h>

thread_local std::default_random_engine g_randomGenerator;

int TerrainArea::g_lodPixelCount[] = { 257, 129, 65, 33, 17, 9, 5, 3 };
int TerrainArea::g_lodSpacing[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
float TerrainArea::g_noiseCurve[] = { 0.5f, 0.2f, 0.073f, 0.038f, 0.017f, 0.009f, 0.0046f, 0.0025f };
float TerrainArea::g_heightAmplitude = 100.f;	// 300m from super high to super low
float TerrainArea::g_seeLevel = 10.f;			// 30m under see is super low
float TerrainArea::g_snowLevel = 250.f;			
float TerrainArea::g_erosion = 2.f;
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
		int n = (int)(x * 311 + y * 113 + g_seed);
		n = (n << 13) ^ n;
		n = n * (n * n * 15731 + 789221) + 1376312589;
		return float(n & 0x0fffffff) / float(0x0fffffff);
	};
	const auto noised = [hash](float x, float y, int octaves)
	{
		float height = -g_seeLevel;
		vec2f derivative = vec2f(0);
		float interval = 250.f;
		float erosion = g_erosion;

		for (int i = 0; i < octaves; i++, interval *= 0.5f)
		{
			float amplitude = g_heightAmplitude * g_noiseCurve[i];
			float invInterval = 1.f / interval;
			vec2f p = interval * vec2f((int)((x + 0.5f) * invInterval), (int)((y + 0.5f) * invInterval));
			vec2f t = vec2f((x - p.x) * invInterval, (y - p.y) * invInterval);
			vec2f u = t * t * t * (t * (t * 6.f - vec2f(15.f)) + vec2f(10.f));
			vec2f du = 30.f * t * t * (t * (t - vec2f(2.f)) + vec2f(1.f));
			vec2f ddu = 60.f * t * (2.f * t * t - 3.f * t + vec2f(1.f));

			float a = hash(p.x, p.y);
			float b = hash(p.x + interval, p.y);
			float c = hash(p.x, p.y + interval);
			float d = hash(p.x + interval, p.y + interval);

			vec2f lambda = vec2f(b - a, c - a);
			float mu = (a - b - c + d);

			float n = (a + (b - a) * u.x + (c - a) * u.y + mu * u.x * u.y);
			//if (i == 0) erosion = lerp(0.f, g_erosion, n);

			n *= amplitude;
			vec2f dn = (invInterval * amplitude) * (du * (lambda + mu * vec2f(u.y, u.x)));
			vec2f ddn = (invInterval *invInterval * amplitude) * (ddu * (lambda + mu * vec2f(u.y, u.x)) + mu * vec2f(du.x * du.y));
			float denominator = 1.f + erosion * dn.getNorm2();

			height += n / denominator;
			derivative += dn / denominator - (2 * erosion * n) / (denominator * denominator) * (dn * ddn);
		}
		return vec3f(height - g_seeLevel, derivative.x, derivative.y);
	};
	const auto materialSelect = [&](const MapData& data, int i, int j)
	{
		if (data.normalTerrain.y < 0.85f)
			return 9;//cliff

		float height = data.heightTerrain + 3.f * hash((float)i, (float)j);
		if (std::min(height, data.heightTerrain) < 0.1)
			return (hash((float)j, (float)i) > 0.2f ? 4 : 5);    // sand
		else if (std::min(height, data.heightTerrain) < 0.7)
			return 7;    // dirt
		else if (height > g_snowLevel)
			return 8;	  // snow
		return 0; // grass
	};

	// direct write noised and derivative
	constexpr float faceScale = 250.f / 256.f;
	MapData** data = new MapData*[257];
	for (int i = 0; i < 257; i++)
		data[i] = new MapData[257];
	for (int i = 0; i < 257; i++)
		for (int j = 0; j < 257; j++)
		{
			vec3f n = noised(m_gridIndex.x * 250 + faceScale * i, m_gridIndex.y * 250 + faceScale * j, 8);
			data[i][j].heightTerrain = clamp(n.x, -g_seeLevel, g_heightAmplitude - g_seeLevel);
			data[i][j].heightWater = 0.f;
			data[i][j].holeTerrain = false;
			data[i][j].holeWater = data[i][j].heightTerrain > data[i][j].heightWater + 2.f;
			data[i][j].normalTerrain = vec4f(n.y, 2, n.z, 0).getNormal();
			data[i][j].normalWater = vec4f(0, 1, 0, 0);
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

	// details
	DetailInstanceDataHeader header;
	std::vector<std::vector<vec4ui>> instanceDatas;
	auto& areaDetails = m_terrain->getAreaDetails();
	int identifier = 0;
	for (const auto& areaDetail : areaDetails)
	{
		for (int k = 0; k < areaDetail.m_meshNames.size(); k++)
		{
			std::vector<vec4ui> insdata = generateDetails(areaDetail.m_density, areaDetail.m_probability[k], areaDetail.m_sizeRange, areaDetail.m_allowedMaterials, texture.data());
			if (insdata.size() > 0)
			{
				header.m_arrayIdentifier.push_back(identifier);
				header.m_arraySizes.push_back((int)insdata.size());
				instanceDatas.push_back(insdata);
			}
			identifier++;
		}
	}
	header.m_arrayCount = (int)header.m_arrayIdentifier.size();

	// save to file
	filename = getNameFromIndex() + ".areaDetails";
	std::ofstream file2(directory + filename, std::ios::out | std::ios::binary);
	if (file2)
	{
		// header
		file2.write((const char*)&header.m_arrayCount, sizeof(int));
		for (int i = 0; i < header.m_arrayCount; i++)
			file2.write((const char*)&header.m_arrayIdentifier[i], sizeof(int));
		for (int i = 0; i < header.m_arrayCount; i++)
			file2.write((const char*)&header.m_arraySizes[i], sizeof(int));

		// datas
		for (int i = 0; i < header.m_arrayCount; i++)
			file2.write((const char*)instanceDatas[i].data(), instanceDatas[i].size() * sizeof(vec4ui));

		file2.close();
	}
	else
	{
		std::cout << ConsoleColor::red << "[ERROR] : cannot create file " << filename << ConsoleColor::classic << std::endl;
	}

	// clear all
	for (int i = 0; i < 257; i++)
		delete[] data[i];
	delete[] data;
}
std::vector<vec4ui> TerrainArea::generateDetails(float density, vec2f probability, vec2f scaleRange, const std::vector<int>& allowedMaterials, const uint64_t* dataPtr)
{
	const auto hash3 = [](float x, float y)
	{
		vec3f res;
		int n = (int)(x * 311 + y * 113 + g_seed);
		n = (n << 13) ^ n;
		n = n * (n * n * 15731 + 789221) + 1376312589;
		res.x = float(n & 0x0fffffff) / float(0x0fffffff);

		n = (int)(x * 313 + y * 109 + g_seed);
		n = (n << 13) ^ n;
		n = n * (n * n * 15731 + 789221) + 1376312589;
		res.y = float(n & 0x0fffffff) / float(0x0fffffff);

		n = (int)(x * 317 + y * 107 + g_seed);
		n = (n << 13) ^ n;
		n = n * (n * n * 15731 + 789221) + 1376312589;
		res.z = float(n & 0x0fffffff) / float(0x0fffffff);

		return res;
	};
	const auto biLerpv = [](vec4f a, vec4f b, vec4f c, vec4f d, float s, float t)
	{
		vec4f x = vec4f::lerp(a, b, t);
		vec4f y = vec4f::lerp(c, d, t);
		return vec4f::lerp(x, y, s);
	};
	const auto biLerpf = [](float a, float b, float c, float d, float s, float t)
	{
		float x = lerp(a, b, t);
		float y = lerp(c, d, t);
		return lerp(x, y, s);
	};
	const auto isAllowedMaterial = [allowedMaterials](int mat)
	{
		for (int m : allowedMaterials)
			if (m == mat)
				return true;
		return false;
	};

	std::vector<vec4ui> instanceDatas;
	const float displacementRange = 0.49f;
	const float fullModelScale = 10.f;
	const float pi2 = 2.f * (float)PI;
	const uint64_t* data = dataPtr ? dataPtr : m_data;

	int gridPlacement = std::clamp((int)(250.f / std::max(0.001f, density)), 1, 0xFFFF);
	instanceDatas.reserve(gridPlacement * gridPlacement);
	float spacing = 250.f / gridPlacement;
	vec2f posOffset = vec2f(0.5f * spacing - 125.f);
	vec2f c = vec2f(getCenter().x, getCenter().z);
	for (int i = 0; i < gridPlacement; i++)
		for (int j = 0; j < gridPlacement; j++)
		{
			vec2f pos = posOffset + vec2f(i * spacing, j * spacing);
			vec3f random = hash3(pos.x + c.x, pos.y + c.y);
			if (random.x <= probability.x || random.x > probability.y)
				continue;

			pos.x += lerp(-displacementRange * spacing, displacementRange * spacing, random.y);
			pos.y += lerp(-displacementRange * spacing, displacementRange * spacing, random.z);
			vec2f tileuv = clamp((1.f / 250.f) * (pos + vec2f(125.f)), vec2f(0.f), vec2f(1.f));

			int iposx = clamp((int)(tileuv.x * 256), 0, 256);
			int iposy = clamp((int)(tileuv.y * 256), 0, 256);
			MapData data0; data0.unpack(data[257 * iposx + iposy]);
			MapData data1; data1.unpack(data[257 * (iposx + 1) + iposy]);
			MapData data2; data2.unpack(data[257 * iposx + iposy + 1]);
			MapData data3; data3.unpack(data[257 * (iposx + 1) + iposy + 1]);

			// discard all patch not on allowed materials
			if (data0.material == data1.material && data0.material == data2.material && data0.material == data3.material)
			{
				if (!isAllowedMaterial(data0.material))
					continue;
			}
			else
			{
				if (!isAllowedMaterial(data0.material) || !isAllowedMaterial(data1.material) || !isAllowedMaterial(data2.material) || !isAllowedMaterial(data3.material))
					continue;
			}

			float uvx = clamp(tileuv.x * 256 - iposx, 0.f, 1.f);
			float uvy = clamp(tileuv.y * 256 - iposy, 0.f, 1.f);
			//float heightTerrain = biLerpf(data0.heightTerrain, data1.heightTerrain, data2.heightTerrain, data3.heightTerrain, uvx, uvy);
			//float heightWater = biLerpf(data0.heightWater, data1.heightWater, data2.heightWater, data3.heightWater, uvx, uvy);

			vec4f normal = biLerpv(data0.normalTerrain, data1.normalTerrain, data2.normalTerrain, data3.normalTerrain, uvx, uvy).getNormal();

			// discard too stiff slopes
			if (normal.y < 0.95f)
				continue;

			vec3f random2 = hash3(random.x, random.y);
			float scale = lerp(scaleRange.x, scaleRange.y, random2.x);
			float angle = pi2 * random2.y;
			float tint = random2.z;

			uint32_t x = 65535 * std::clamp(pos.x + 125.f, 0.f, 250.f) / 250.f;
			uint32_t z = 65535 * std::clamp(pos.y + 125.f, 0.f, 250.f) / 250.f;
			uint32_t s = 65535 * std::clamp(scale, 0.f, fullModelScale) / fullModelScale;
			uint32_t a = 65535 * std::clamp(angle, 0.f, pi2) / pi2;
			uint32_t t = 511 * tint;
			instanceDatas.push_back(vec4ui((x << 16) | z, (a << 16) | s, t, 0));
		}

	return instanceDatas;
}
Entity* TerrainArea::addDetailsInstance(const std::string& meshName, float density, const std::vector<int>& allowedMaterials, vec2f probability, vec2f scaleRange, float worldNormalWeight, float modelOffset)
{
	GF_ASSERT(m_data, "No data !");

	ResourceManager* resmgr = ResourceManager::getInstance();
	std::vector<vec4ui> instanceDatas = generateDetails(density, probability, scaleRange, allowedMaterials);
	const float fullModelScale = 10.f;

	Entity* massInstance = m_entity->getParentWorld()->getEntityFactory().createObject([&](Entity* object)
		{
			object->setName("MassInstancing");
			object->setWorldPosition(getCenter());

			TerrainDetailDrawableComponent* drawable = new TerrainDetailDrawableComponent(this);
			drawable->setMaterial(resmgr->getResource<Material>("terrainDetail"));
			drawable->setMesh(resmgr->getResource<Mesh>(meshName));
			drawable->setInstanceData(instanceDatas);
			drawable->setFullModelScale(fullModelScale);
			drawable->setWorldNormalWeight(worldNormalWeight);
			drawable->setModelOffset(modelOffset);
			drawable->initializeVBO();
			drawable->initializeVAO();
			object->addComponent(drawable);

			object->recomputeBoundingBox();
		}, false);

	m_entity->addChild(massInstance);
	massInstance->setLocalPosition(vec4f(0.f));
	m_entity->getParentWorld()->addToScene(massInstance);
	return massInstance;
}
Entity* TerrainArea::addDetailsInstance(Mesh* mesh, int identifier, float worldNormalWeight, float modelOffset)
{
	const float fullModelScale = 10.f;

	std::vector<vec4ui> instanceDatas;
	for (int i = 0; i < m_instanceHeader.m_arrayIdentifier.size(); i++)
	{
		if (m_instanceHeader.m_arrayIdentifier[i] == identifier)
		{
			instanceDatas = m_instanceDatas[i];
			break;
		}
	}

	Entity* massInstance = m_entity->getParentWorld()->getEntityFactory().createObject([&](Entity* object)
		{
			object->setName("MassInstancing id" + std::to_string(identifier));
			object->setWorldPosition(getCenter());

			TerrainDetailDrawableComponent* drawable = new TerrainDetailDrawableComponent(this);
			drawable->setMaterial(m_terrain->getDetailMaterial());


			drawable->setMesh(mesh);
			drawable->setInstanceData(instanceDatas);
			drawable->setFullModelScale(fullModelScale);
			drawable->setWorldNormalWeight(worldNormalWeight);
			drawable->setModelOffset(modelOffset);
			drawable->initializeVBO();
			drawable->initializeVAO();
			object->addComponent(drawable);

			object->recomputeBoundingBox();
		}, false);

	m_entity->addChild(massInstance);
	massInstance->setLocalPosition(vec4f(0.f));
	m_entity->getParentWorld()->addToScene(massInstance);
	return massInstance;
}

void TerrainArea::loadInstanceData()
{
	if (!m_instanceDatas.empty())
		return;

	std::string filename = getNameFromIndex() + ".areaDetails";
	std::ifstream file(m_terrain->getDirectory() + "/" + filename, std::ios::binary);
	//GF_ASSERT(file, "No instance datas file found !");

	if (file)
	{
		file.read((char*)&m_instanceHeader.m_arrayCount, sizeof(int));
		for (int i = 0; i < m_instanceHeader.m_arrayCount; i++)
		{
			int tmpInteger;
			file.read((char*)&tmpInteger, sizeof(int));
			m_instanceHeader.m_arrayIdentifier.push_back(tmpInteger);
		}
		for (int i = 0; i < m_instanceHeader.m_arrayCount; i++)
		{
			int tmpInteger;
			file.read((char*)&tmpInteger, sizeof(int));
			m_instanceHeader.m_arraySizes.push_back(tmpInteger);
		}
		for (int i = 0; i < m_instanceHeader.m_arrayCount; i++)
		{
			m_instanceDatas.emplace_back();
			m_instanceDatas.back().insert(m_instanceDatas.back().begin(), m_instanceHeader.m_arraySizes[i], vec4ui::zero);
			file.read((char*)m_instanceDatas.back().data(), m_instanceHeader.m_arraySizes[i] * sizeof(vec4ui));
		}
		file.close();
	}
}
void TerrainArea::unloadInstanceData()
{
	m_instanceDatas.clear();
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
		SCOPED_CPU_MARKER("TerrainArea::loadHeightmap");
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
			if (data.heightWater > data.heightTerrain)
				m_hasWater = true;

			minHeight = std::min(minHeight, data.heightWater);
			minHeight = std::min(minHeight, data.heightTerrain);
			maxHeight = std::max(maxHeight, data.heightWater);
			maxHeight = std::max(maxHeight, data.heightTerrain);
		}

		m_boundingBox.min = vec4f(-125.f, minHeight, -125.f, 0.f);
		m_boundingBox.max = vec4f( 125.f, maxHeight,  125.f, 0.f);
		if (m_entity)
			m_entity->recomputeBoundingBox();
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
const TerrainVirtualTexture::TextureTile& TerrainArea::getTileData(int lod) const
{
	return m_tiles[lod];
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

	uint64_t theight = (uint64_t)((uint16_t)((heightTerrain + g_seeLevel) * scaleFactor));
	uint64_t wheight = (uint64_t)((uint16_t)((heightWater + g_seeLevel) * scaleFactor));
	uint64_t tnormal = octahedralPack(normalTerrain, 7);
	uint64_t wnormal = octahedralPack(normalWater, 5);
	uint64_t mat = (uint64_t)material;
	uint64_t thole = holeTerrain ? 0x01 : 0x00;
	uint64_t whole = holeWater ? 0x01 : 0x00;

	return theight | (wheight << 16) | (tnormal << 32) | (thole << 46) | (whole << 47) | (wnormal << 48) | (mat << 58);
}
void TerrainArea::MapData::unpack(uint64_t data)
{
	float scaleFactor = TerrainArea::g_heightAmplitude / 65535.f;

	heightTerrain = (data & 0xFFFF) * scaleFactor - g_seeLevel;
	heightWater = ((data >> 16) & 0xFFFF) * scaleFactor - g_seeLevel;
	normalTerrain = octahedralUnpack(data >> 32, 7);
	normalWater = octahedralUnpack(data >> 48, 5);
	material = data >> 58;
	holeTerrain = (data & ((uint64_t)1 << 46));
	holeWater = (data & ((uint64_t)1 << 47));
}

uint64_t TerrainArea::MapData::octahedralPack(vec4f n, int bits)
{
	// octahedral packing, exept that normal.y is always pointing up, so no copysign
	n /= std::abs(n.x) + std::abs(n.y) + std::abs(n.z);
	n = vec4f(0.5f) + vec4f(0.5f) * n;
	int mask = (1 << bits) - 1;
	vec2i d = vec2i((int)(n.x * mask + 0.5f), (int)(n.z * mask + 0.5f));
	return ((uint64_t)d.y << bits) | d.x;
}
vec4f TerrainArea::MapData::octahedralUnpack(uint64_t n, int bits)
{
	int mask = (1 << bits) - 1;
	vec2i d = vec2i(n & mask, (n >> bits) & mask);
	vec4f v = vec4f(1.f - 2.f * d.x / mask, 0, 1.f - 2.f * d.y / mask, 0);
	v.y = 1.f - std::abs(v.x) - std::abs(v.z);
	return v.getNormal();
}