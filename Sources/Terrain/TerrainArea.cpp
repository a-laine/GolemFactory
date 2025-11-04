#include "TerrainArea.h"
#include "Terrain.h"
#include "TerrainDetailDrawableComponent.h"

#include <Utiles/ConsoleColor.h>
#include <Utiles/ProfilerConfig.h>
#include <random>
#include <Utiles/Assert.hpp>
#include <Resources/ResourceManager.h>
#include <Physics/Collision.h>

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
		float interval = AREA_WORLDSCALE;
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
		float height = data.height + 3.f * hash((float)i, (float)j);
		if (std::min(height, data.height) < 0.1)
			return (hash((float)j, (float)i) > 0.2f ? 4 : 5);    // sand
		else if (std::min(height, data.height) < 0.7)
			return 7;    // dirt
		else if (height > g_snowLevel)
			return 8;	  // snow
		return 0; // grass
	};

	// direct write noised and derivative
	constexpr float faceScale = AREA_WORLDSCALE / 256.f;
	MapData** data = new MapData*[257];
	for (int i = 0; i < 257; i++)
		data[i] = new MapData[257];
	for (int i = 0; i < 257; i++)
		for (int j = 0; j < 257; j++)
		{
			vec3f n = noised(m_gridIndex.x * AREA_WORLDSCALE + faceScale * i, m_gridIndex.y * AREA_WORLDSCALE + faceScale * j, 8);
			data[i][j].height = clamp(n.x, -g_seeLevel, g_heightAmplitude - g_seeLevel);
			data[i][j].hole = false;
			data[i][j].normal = vec4f(n.y, 2, n.z, 0).getNormal();
			data[i][j].material0 = materialSelect(data[i][j], 256 * m_gridIndex.x + i, 256 * m_gridIndex.y + j);

			float cliffFactor = smoothstep(0.75f, 0.9f, data[i][j].normal.y);
			if (cliffFactor < 0.01f)
			{
				data[i][j].material0 = 9;
				data[i][j].material1 = 0xFF;
				data[i][j].material0weight = 1.f;
			}
			else if (cliffFactor > 0.99f)
			{
				data[i][j].material1 = 0xFF;
				data[i][j].material0weight = 1.f;
			}
			else
			{
				data[i][j].material1 = 9;
				data[i][j].material0weight = cliffFactor;
			}
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
	for (int i = 0; i < areaDetails.size(); i++)//const auto& areaDetail : areaDetails)
	{
		const Terrain::AreaDetails& detail = areaDetails[i];
		for (int k = 0; k < detail.m_meshNames.size(); k++)
		{
			std::vector<vec4ui> insdata = generateDetails(detail.m_density, detail.m_probability[k], detail.m_sizeRange, detail.m_allowedMaterials, texture.data());
			if (insdata.size() > 0)
			{
				header.m_arrayIdentifier.push_back((i << 16) | k);
				header.m_arraySizes.push_back((int)insdata.size());
				instanceDatas.push_back(insdata);
			}
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

	int gridPlacement = std::clamp((int)(AREA_WORLDSCALE / std::max(0.001f, density)), 1, 0xFFFF);
	instanceDatas.reserve(gridPlacement * gridPlacement);
	float spacing = AREA_WORLDSCALE / gridPlacement;
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
			vec2f tileuv = clamp((1.f / AREA_WORLDSCALE) * (pos + vec2f(0.5f * AREA_WORLDSCALE)), vec2f(0.f), vec2f(1.f));

			int iposx = clamp((int)(tileuv.x * 256), 0, 256);
			int iposy = clamp((int)(tileuv.y * 256), 0, 256);
			float uvx = clamp(tileuv.x * 256 - iposx, 0.f, 1.f);
			float uvy = clamp(tileuv.y * 256 - iposy, 0.f, 1.f);

			MapData data0; data0.unpack(data[257 * iposx + iposy]);
			MapData data1; data1.unpack(data[257 * (iposx + 1) + iposy]);
			MapData data2; data2.unpack(data[257 * iposx + iposy + 1]);
			MapData data3; data3.unpack(data[257 * (iposx + 1) + iposy + 1]);

			vec4f normal = biLerpv(data0.normal, data1.normal, data2.normal, data3.normal, uvx, uvy).getNormal();

			// discard too stiff slopes
			if (normal.y < 0.95f)
				continue;

			// discard all patch not on allowed materials
			float proba0 = (isAllowedMaterial(data0.material0) ? data0.material0weight : 0.f) + (isAllowedMaterial(data0.material1) ? 1.f - data0.material0weight : 0.f);
			float proba1 = (isAllowedMaterial(data1.material0) ? data1.material0weight : 0.f) + (isAllowedMaterial(data1.material1) ? 1.f - data1.material0weight : 0.f);
			float proba2 = (isAllowedMaterial(data2.material0) ? data2.material0weight : 0.f) + (isAllowedMaterial(data2.material1) ? 1.f - data2.material0weight : 0.f);
			float proba3 = (isAllowedMaterial(data3.material0) ? data3.material0weight : 0.f) + (isAllowedMaterial(data3.material1) ? 1.f - data3.material0weight : 0.f);
			float matProba = biLerpf(proba0, proba1, proba2, proba3, uvx, uvy);
			if (matProba < 0.01f)
				continue;

			vec3f random2 = hash3(random.x, random.y);
			float scale = lerp(scaleRange.x, scaleRange.y, random2.x);
			float angle = pi2 * random2.y;
			float tint = random2.z;

			uint32_t x = 65535 * std::clamp(pos.x + 0.5f * AREA_WORLDSCALE, 0.f, AREA_WORLDSCALE) / AREA_WORLDSCALE;
			uint32_t z = 65535 * std::clamp(pos.y + 0.5f * AREA_WORLDSCALE, 0.f, AREA_WORLDSCALE) / AREA_WORLDSCALE;
			uint32_t s = 65535 * std::clamp(scale, 0.f, fullModelScale) / fullModelScale;
			uint32_t a = 65535 * std::clamp(angle, 0.f, pi2) / pi2;
			uint32_t t = 511 * tint;
			instanceDatas.push_back(vec4ui((x << 16) | z, (a << 16) | s, t, 0));
		}

	return instanceDatas;
}
bool TerrainArea::hasDataForDetail(int identifier) const
{
	for (int i = 0; i < m_instanceHeader.m_arrayIdentifier.size(); i++)
	{
		if (m_instanceHeader.m_arrayIdentifier[i] == identifier)
			return !m_instanceDatas[i].empty();
	}
	return false;
}
Entity* TerrainArea::addDetailsInstance(const std::string& meshName, float density, const std::vector<int>& allowedMaterials, vec2f probability, vec2f scaleRange, float worldNormalWeight, float modelOffset)
{
	GF_ASSERT_MSG(m_data, "No data !");

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
Entity* TerrainArea::addDetailsInstance(Mesh* mesh, int identifier)
{
	const float fullModelScale = 10.f;

	std::vector<vec4ui>* instanceDatas = nullptr;
	for (int i = 0; i < m_instanceHeader.m_arrayIdentifier.size(); i++)
	{
		if (m_instanceHeader.m_arrayIdentifier[i] == identifier)
		{
			instanceDatas = &m_instanceDatas[i];
			break;
		}
	}
	if (!instanceDatas)
		return nullptr;

	const Terrain::AreaDetails& detail = getTerrain()->getAreaDetails()[identifier >> 16];
	const std::string detailName = detail.m_name;
	Entity* massInstance = m_entity->getParentWorld()->getEntityFactory().createObject([&](Entity* object)
		{
			object->setName("MassInstancing " + detailName + " mesh" + std::to_string(identifier & 0xFFFF));
			object->setWorldPosition(getCenter());

			TerrainDetailDrawableComponent* drawable = new TerrainDetailDrawableComponent(this);
			drawable->setMaterial(m_terrain->getDetailMaterial());

			drawable->setMesh(mesh);
			drawable->setInstanceData(*instanceDatas);
			drawable->setFullModelScale(fullModelScale);
			drawable->setWorldNormalWeight(detail.m_normalWeight);
			drawable->setModelOffset(detail.m_modelOffset[identifier & 0xFFFF]);
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
	//GF_ASSERT_MSG(file, "No instance datas file found !");

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
		GF_ASSERT_MSG(file, "No heightmap file found !");

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
			minHeight = std::min(minHeight, data.height);
			maxHeight = std::max(maxHeight, data.height);
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
					GF_ASSERT_MSG(m_tiles[l].m_lod >= 0, "No virtual texture tile found !");

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
const AxisAlignedBox& TerrainArea::getBoundingBox() const
{
	return m_boundingBox;
}
vec2i TerrainArea::getGridIndex() const
{
	return m_gridIndex;
}


bool TerrainArea::getCollisionInCache(Physics::CollisionCache& cache) const
{
	if (!m_data)
		return false;

	if (!Collision::collide_AxisAlignedBoxvsAxisAlignedBox(cache.m_aabb.min, cache.m_aabb.max, m_center + m_boundingBox.min, m_center + m_boundingBox.max))
		return false;

	vec4f corner = m_center - vec4f(0.5f * AREA_WORLDSCALE, 0, 0.5f * AREA_WORLDSCALE, 0);
	corner.y = 0;
	corner.w = 1;
	float invScale = 1.f / AREA_WORLDSCALE;
	float id2world = AREA_WORLDSCALE / 256;
	vec4f world2index = vec4f(256 * invScale);
	vec4f m = (cache.m_aabb.min - corner) * world2index;
	vec4f M = (cache.m_aabb.max - corner) * world2index;
	vec2i min = vec2i::clamp(vec2i((int)m.x, (int)m.z), vec2i::zero, vec2i(255));
	vec2i max = vec2i::clamp(vec2i((int)M.x, (int)M.z), vec2i::zero, vec2i(255));
	bool hasCollision = false;

	MapData data0, data1, data2, data3;
	for (int i = min.x; i <= max.x; i++)
		for (int j = min.y; j <= max.y; j++)
		{
			data0.unpack(m_data[i * 257 + j]);
			data1.unpack(m_data[i * 257 + j + 1]);
			data2.unpack(m_data[(i + 1) * 257 + j + 1]);
			data3.unpack(m_data[(i + 1) * 257 + j]);

			Triangle t0, t1;
			AxisAlignedBox quadaabb;
			if (((i + j) & 0x01) == 0)
			{
				t0.p1 = t1.p1 = corner + vec4f(i * id2world, data0.height, j * id2world, 0);
				t0.p2 = corner + vec4f(i * id2world, data1.height, (j + 1) * id2world, 0);
				t0.p3 = t1.p2 = corner + vec4f((i + 1) * id2world, data2.height, (j + 1) * id2world, 0);
				t1.p3 = corner + vec4f((i + 1) * id2world, data3.height, j * id2world, 0);
				quadaabb.min = t0.p1;
				quadaabb.max = t0.p3;
			}
			else
			{
				t0.p1 = corner + vec4f(i * id2world, data0.height, j * id2world, 0);
				t0.p2 = t1.p1 = corner + vec4f(i * id2world, data1.height, (j + 1) * id2world, 0);
				t1.p2 = corner + vec4f((i + 1) * id2world, data2.height, (j + 1) * id2world, 0);
				t0.p3 = t1.p3 = corner + vec4f((i + 1) * id2world, data3.height, j * id2world, 0);
				quadaabb.min = t0.p1;
				quadaabb.max = t1.p2;
			}

			quadaabb.min.y = std::min(std::min(data0.height, data1.height), std::min(data2.height, data3.height));
			quadaabb.max.y = std::max(std::max(data0.height, data1.height), std::max(data2.height, data3.height));

			if (Collision::collide(&cache.m_aabb, &quadaabb))
			{
				Physics::CollisionCache::Element element;
				element.m_entity = m_entity;
				hasCollision = true;

				element.m_shape = &(cache.m_triangles[cache.m_triangles.add(t0)]);
				cache.m_elements.push_back(element);

				element.m_shape = &(cache.m_triangles[cache.m_triangles.add(t1)]);
				cache.m_elements.push_back(element);
			}
		}
	return hasCollision;
}
//

uint64_t TerrainArea::MapData::pack()
{
	float scaleFactor = 65535.f / TerrainArea::g_heightAmplitude;

	uint64_t theight = (uint64_t)((uint16_t)((height + g_seeLevel) * scaleFactor));
	uint64_t tnormal = octahedralPack(normal, 8);
	uint64_t mat0 = (uint64_t)material0;
	uint64_t mat1 = (uint64_t)material1;
	uint64_t wmat0 = (uint64_t)(255 * material0weight);
	uint64_t h = hole ? 0x01 : 0x00;

	/*
	_________________________________
	|F|E|D|C|B|A|9|8|7|6|5|4|3|2|1|0|
	=================================

	_________________________________
	|            height             |
	=================================
	|     normal1   |     normal0   |
	=================================
	|   material1   |   material0   |
	=================================
	|h|             |   matWeight0  |  h=hole
	=================================
	*/

	return theight | (tnormal << 16) | (mat0 << 32) | (mat1 << 40) | (wmat0 << 48) | (h << 63);
}
void TerrainArea::MapData::unpack(uint64_t data)
{
	float scaleFactor = TerrainArea::g_heightAmplitude / 65535.f;

	height = (data & 0xFFFF) * scaleFactor - g_seeLevel;
	normal = octahedralUnpack(data >> 16, 8);
	material0 = (data >> 32) & 0xFF;
	material1 = (data >> 40) & 0xFF;
	material0weight = ((data >> 48) & 0xFF) * (1.f / 255.f);
	hole = (data & ((uint64_t)1 << 63));
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