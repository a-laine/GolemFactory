#include "Terrain.h"
#include "TerrainAreaDrawableComponent.h"

#include <Resources/ResourceManager.h>
#include <World/World.h>
#include <World/WorldComponents/EntityFactory.h>
#include <Utiles/ProfilerConfig.h>

#include <filesystem>

#ifdef USE_IMGUI
	#define IMGUI_DEFINE_MATH_OPERATORS
	#include <imgui_internal.h>
	bool TerrainWindowEnable = true;
#endif
#include <Utiles/Debug.h>
#include <Terrain/TerrainDetailDrawableComponent.h>
#include <Resources/Material.h>

float Terrain::g_morphingRange = 10.f;


// Default
Terrain::Terrain() : m_world(nullptr), m_areaContainer(nullptr), m_virtualTexture(nullptr), m_gridSize(0), 
	m_grid(nullptr), m_terrainMaterial(nullptr), m_waterMaterial(nullptr), m_terrainDetailMaterial(nullptr)
{
	m_gridMinIndex = vec2i(-std::numeric_limits<int>::max());
}
Terrain::~Terrain()
{
	m_areas.clear();
	recomputeGrid();

	m_clipmapMeshes.clear();

	ResourceManager::getInstance()->release(m_terrainMaterial);
	ResourceManager::getInstance()->release(m_waterMaterial);
	ResourceManager::getInstance()->release(m_terrainDetailMaterial);
}
//

// public functions
void Terrain::initializeClipmaps()
{
	const int lodFaceCount[] = { 256, 128, 64, 32, 16, 8, 4, 2 };
	constexpr int lodCount = sizeof(lodFaceCount) / sizeof(int);
	constexpr float faceScale = 250.f;

	std::vector<vec4f> vertices;
	std::vector<vec4f> uvs;
	std::vector<vec4f> dummyArrayVec4f;
	std::vector<vec4i> dummyArrayVec4i;
	std::vector<uint32_t> indices;
	vertices.reserve((lodFaceCount[0] + 1) * (lodFaceCount[0] + 1));
	indices.reserve(lodFaceCount[0] * lodFaceCount[0] * 6);

	m_clipmapMeshes.reserve(lodCount);
	vec4f verticalMorph = vec4f(0, 0, 1, 0);
	vec4f horizontalMorph = vec4f(0, 0, 0, 1);
	vec4f diagAMorph = vec4f(0, 0, 1, 1);
	vec4f diagBMorph = vec4f(0, 0, -1, 1);

	for (int lod = 0; lod < lodCount; lod++)
	{
		// allocate
		int faceCount = lodFaceCount[lod];
		vec2f faceSize = vec2f(1.f / faceCount);
		vertices.clear();
		indices.clear();
		uvs.clear();

		// push vertices
		for (int i = 0; i <= faceCount; i++)
			for (int j = 0; j <= faceCount; j++)
			{
				vertices.push_back(vec4f(
					faceScale * (i * faceSize.x -0.5f), 
					(faceScale * 0.003f / faceCount) * (rand() & 0xFF), 
					faceScale * (j * faceSize.y - 0.5f), 1.f)
				);

				// j, i because of opengl index flip
				uvs.push_back(vec4f(j, i, 0, 0));
			}

		int vertexCount = faceCount + 1;
		int k = 0;
		for (int i = 1; i < faceCount; i += 2, k++)
		{
			int l = 0;
			for (int j = 1; j < faceCount; j += 2, l++)
			{
				bool odd = (k + l) & 0x01;
				uvs[i * vertexCount + j] += odd ? diagBMorph : diagAMorph;
				uvs[(i - 1) * vertexCount + j] += verticalMorph;
				uvs[i * vertexCount + (j - 1)] += horizontalMorph;
				if (i == faceCount - 1)
					uvs[(i + 1) * vertexCount + j] += verticalMorph;
				if (j == faceCount - 1)
					uvs[i * vertexCount + (j + 1)] += horizontalMorph;
			}
		}


		// push indices
		for (int i = 0; i < faceCount; i++)
			for (int j = 0; j < faceCount; j++)
			{
				uint32_t i0 = i * (faceCount + 1) + j;
				uint32_t i1 = (i + 1) * (faceCount + 1) + j;
				uint32_t i2 = (i + 1) * (faceCount + 1) + j + 1;
				uint32_t i3 = i * (faceCount + 1) + j + 1;

				bool evenFaceIndex = (((i + j) & 0x01) == 0);
				if (evenFaceIndex)
				{
					indices.push_back(i0);
					indices.push_back(i1);
					indices.push_back(i2);
					indices.push_back(i0);
					indices.push_back(i2);
					indices.push_back(i3);
				}
				else
				{
					indices.push_back(i0);
					indices.push_back(i1);
					indices.push_back(i3);
					indices.push_back(i1);
					indices.push_back(i2);
					indices.push_back(i3);
				}
			}

		Mesh* clipmap = new Mesh("internalClipmapMeshLod" + std::to_string(lod));
		clipmap->initialize(vertices, dummyArrayVec4f, uvs, indices, dummyArrayVec4i, dummyArrayVec4f);
		clipmap->isEnginePrivate = true;
		ResourceManager::getInstance()->addResource(clipmap);
		m_clipmapMeshes.push_back(clipmap);
	}

	m_terrainMaterial = ResourceManager::getInstance()->getResource<Material>("terrain");
	m_waterMaterial = ResourceManager::getInstance()->getResource<Material>("terrainWater");
	m_terrainDetailMaterial = ResourceManager::getInstance()->getResource<Material>("terrainDetail");

	if (!m_areaContainer)
	{
		m_areaContainer = m_world->getEntityFactory().createObject([&](Entity* object)
			{
				object->setName("TerrainAreaContainer");
				object->setWorldPosition(vec4f(0, 0, 0, 1));
			});
		m_world->getSceneManager().addToRootList(m_areaContainer);
	}
}

Terrain::AreaDetails& Terrain::addDetail()
{
	m_areaDetails.push_back(AreaDetails());
	return m_areaDetails.back();
}
void Terrain::endAddDetail()
{
	ResourceManager* resmgr = ResourceManager::getInstance();
	int identifier = 0;
	for (auto& areaDetail : m_areaDetails)
	{
		for (int i = 0; i < areaDetail.m_meshNames.size(); i++)
		{
			areaDetail.m_meshResources.push_back(resmgr->getResource<Mesh>(areaDetail.m_meshNames[i]));
			areaDetail.m_identifiers.push_back(identifier + i);
		}
		identifier += (int)areaDetail.m_meshNames.size();
	}
}

void Terrain::addArea(TerrainArea& area, bool recomputeIfNeeded)
{
	vec2i_ordered key(area.m_areaIndex);
	if (m_areas.find(key) != m_areas.end())
		return;

	auto it = m_areas.insert({ key, area });

	vec2i gridindex = area.m_areaIndex - m_gridMinIndex;
	if (gridindex.x >= 0 && gridindex.x < m_gridSize.x && gridindex.y >= 0 && gridindex.y < m_gridSize.y)
		m_grid[gridindex.x][gridindex.y] = &it.first->second;
	else if (recomputeIfNeeded)
		recomputeGrid();
}
void Terrain::removeArea(vec2i index)
{

}
void Terrain::clear()
{
	if (m_grid)
	{
		if (m_world && m_areaContainer)
		{
			for (int i = 0; i < m_gridSize.x; i++)
				for (int j = 0; j < m_gridSize.y; j++)
				{
					TerrainArea* area = m_grid[i][j];
					if (area && area->m_entity)
					{
						area->setLod(-1);
						area->m_entity->removeAllChild(true);
						m_areaContainer->removeChild(area->m_entity, true);
					}
				}
		}

		for (int i = 0; i < m_gridSize.x; i++)
			delete[] m_grid[i];
		delete[] m_grid;
		m_grid = nullptr;
	}
	m_areas.clear();
	m_previousAreas.clear();
	m_gridMinIndex = vec2i(-std::numeric_limits<int>::max());
	m_gridSize = vec2i(0);
}
void Terrain::generate(const std::string& directory)
{
	for (int i = 0; i < m_gridSize.x; i++)
		for (int j = 0; j < m_gridSize.y; j++)
			m_grid[i][j]->generate(directory);
}
void Terrain::generate(const std::string& directory, vec2i tileIndex)
{
	if (m_grid[tileIndex.x][tileIndex.y])
		m_grid[tileIndex.x][tileIndex.y]->generate(directory);
}
void Terrain::load(const std::string& directory)
{
	if (!ToolBox::isPathExist(directory)) 
		return;

	m_directory = directory;
	const size_t folderOffset = directory.size() + 1;
	const char* extension = ".area";

	for (const auto& entry : std::filesystem::directory_iterator(directory))
	{
		std::string filepath = entry.path().generic_u8string();
		size_t extensionPos = filepath.find(extension, folderOffset);
		if (extensionPos == std::string::npos)
			continue;

		const int filenameSize = extensionPos - folderOffset;
		std::string filename = filepath.substr(folderOffset, filenameSize);
		size_t separatorPos = filename.find('_');
		if (separatorPos == std::string::npos)
			continue;

		std::string xpos = filename.substr(0, separatorPos);
		std::string zpos = filename.substr(separatorPos + 1, filename.size() - separatorPos);
		if (xpos.size() < 1 || zpos.size() < 1)
			continue;

		vec2i areaIndex;
		if (xpos[0] == 'n')
		{
			xpos[0] = '0';
			areaIndex.x = -std::stoi(xpos);
		}
		else areaIndex.x = std::stoi(xpos);

		if (zpos[0] == 'n')
		{
			zpos[0] = '0';
			areaIndex.y = -std::stoi(zpos);
		}
		else areaIndex.y = std::stoi(zpos);

		//std::cout << filename << " >> " << areaIndex.x << " " << areaIndex.y << std::endl;
		TerrainArea area(areaIndex, this);
		addArea(area, false);
	}
	std::cout << "Terrain loaded : " << directory << std::endl;
	recomputeGrid();

	if (!m_areaContainer)
	{
		m_areaContainer = m_world->getEntityFactory().createObject([&](Entity* object)
			{
				object->setName("TerrainAreaContainer");
				object->setWorldPosition(vec4f(0, 0, 0, 1));
			});
		m_world->getSceneManager().addToRootList(m_areaContainer);
	}
}

void Terrain::addLodRadius(float _radiusIncrement)
{
	if (!m_lodRadius.empty())
		m_lodRadius.push_back(m_lodRadius.back() + _radiusIncrement);
	else
		m_lodRadius.push_back(_radiusIncrement);
}
void Terrain::update(vec4f _cameraPosition)
{
	SCOPED_CPU_MARKER("Terrain::updates");
	if (m_lodRadius.empty() || !m_grid || !m_areaContainer)
		return;

	struct AreaJobData
	{
		TerrainArea* m_area;
		int m_targetLod;
		int priority;
	};
	std::vector<AreaJobData> jobLodDatas;
	std::vector<AreaJobData> jobDetailDatas;

	m_currentPlayerPosInTile.x = (250 * 0.5f * m_gridSize.x + _cameraPosition.x) / 250;
	m_currentPlayerPosInTile.y = (250 * 0.5f * m_gridSize.y + _cameraPosition.z) / 250;
	vec2i camAreaIndex;
	camAreaIndex.x = (int)m_currentPlayerPosInTile.x;
	camAreaIndex.y = (int)m_currentPlayerPosInTile.y;

	int farThs = (int)(m_lodRadius.back() / 250);
	vec2i tileSize = vec2i(farThs);
	vec2i min = vec2i::clamp(camAreaIndex - tileSize, vec2i::zero, m_gridSize - vec2i::one);
	vec2i max = vec2i::clamp(camAreaIndex + tileSize, vec2i::zero, m_gridSize - vec2i::one);
	m_currentStreamingMin = min;
	m_currentStreamingMax = max;

	int detailMaxLod = -1;
	for (const AreaDetails& detail : m_areaDetails)
	{
		detailMaxLod = std::max(detailMaxLod, detail.m_lod);
	}

	for (TerrainArea* area : m_previousAreas)
	{
		if (area->m_gridIndex.x < min.x || area->m_gridIndex.x > max.x || area->m_gridIndex.y < min.y || area->m_gridIndex.y > max.y)
		{
			// texture and lod update
			jobLodDatas.push_back({ area, -1, -100 });

			// clear details
			area->m_details.clear();
			area->m_entity->removeAllChild(true);

			// remove area entity
			m_areaContainer->removeChild(area->m_entity, true);
			area->m_entity = nullptr;
		}
	}
	m_previousAreas.clear();

	vec4f halfSize = vec4f(125.f);
	for (int i = min.x; i <= max.x; i++)
		for (int j = min.y; j <= max.y; j++)
		{
			TerrainArea* area = m_grid[i][j];
			if (!area)
				continue;

			// create entity for the area
			if (!area->m_entity)
			{
				area->m_entity = m_world->getEntityFactory().createObject([&](Entity* object)
					{
						object->setName("TerrainArea_" + area->getNameFromIndex());
						object->setWorldPosition(area->getCenter());

						TerrainAreaDrawableComponent* drawable = new TerrainAreaDrawableComponent(area);
						drawable->setMaterial(m_terrainMaterial);
						//drawable->setWaterMaterial(m_waterMaterial);
						drawable->setMesh(m_clipmapMeshes.back());
						object->addComponent(drawable);

						object->recomputeBoundingBox();
					}, false);

				m_world->addToScene(area->m_entity, 0);
				m_areaContainer->addChild(area->m_entity);
			}
			GF_ASSERT(area->m_entity, "No entity for area !");

			// compute desired lod
			vec4f delta = _cameraPosition - area->m_center;
			delta = vec4f::clamp(delta, -halfSize, halfSize) - delta;
			float distance = std::sqrt(delta.x * delta.x + delta.z * delta.z);

			int lod = (int)m_lodRadius.size() - 1;
			for (int k = 0; k < m_lodRadius.size() - 1; k++)
			{
				if (distance < m_lodRadius[k])
				{
					lod = k;
					break;
				}
			}


			// change area lod
			int currentLod = area->getLod();
			if (currentLod != lod)
			{
				//TerrainAreaDrawableComponent* drawable = area->m_entity->getComponent<TerrainAreaDrawableComponent>();
				//drawable->setMesh(m_clipmapMeshes[lod]);
				int priority = std::min(std::abs(camAreaIndex.x - i), std::abs(camAreaIndex.y - j));
				if (currentLod < lod)
					priority += 100;
				jobLodDatas.push_back({ area, lod, priority });
				jobDetailDatas.push_back({ area, detailMaxLod, 0 });
			}
			m_previousAreas.insert(area);
		}

	if (!jobLodDatas.empty())
	{
		std::sort(jobLodDatas.begin(), jobLodDatas.end(), [](const AreaJobData& a, const AreaJobData& b) { return a.priority < b.priority; });

#if 1
		Job updateLodJob(jobLodDatas.size(), [](void* _data, int _id, int _count)
			{
				SCOPED_CPU_MARKER("TerrainArea::updateLodJob");
				AreaJobData* data = (AreaJobData*)_data;
				TerrainArea* area = data[_id].m_area;
				int targetLod = data[_id].m_targetLod;
				area->setLod(targetLod);
				Entity* entity = area->m_entity;
				if (entity)
				{
					TerrainAreaDrawableComponent* drawable = entity->getComponent<TerrainAreaDrawableComponent>();
					drawable->setMesh(area->getTerrain()->m_clipmapMeshes[targetLod]);
					drawable->updateData(area->m_tiles[targetLod]);
				}
			}, jobLodDatas.data());

		updateLodJob.addToQueue(Job::JobPriority::MEDIUM);
		updateLodJob.waitCompletion(true);
		jobLodDatas.clear();
#else
		for (auto& job : jobLodDatas)
		{
			TerrainArea* area = job.m_area;
			int targetLod = job.m_targetLod;
			area->setLod(targetLod);
			Entity* entity = area->m_entity;
			if (entity)
			{
				TerrainAreaDrawableComponent* drawable = entity->getComponent<TerrainAreaDrawableComponent>();
				drawable->setMesh(m_clipmapMeshes[targetLod]);
				drawable->updateData(area->m_tiles[targetLod]);
			}
		}
#endif
		if (m_virtualTexture)
			m_virtualTexture->updateGPUTexture();
	}

	if (!jobDetailDatas.empty())
	{
		// cannot use job for now : we need main thread to init VBO & VAO
#if 0
		Job updateDetailJob(jobDetailDatas.size(), [](void* _data, int _id, int _count)
			{
				SCOPED_CPU_MARKER("TerrainArea::updateDetailJob");
				AreaJobData* data = (AreaJobData*)_data;
				TerrainArea* area = data[_id].m_area;
				int detailMaxLod = data[_id].m_targetLod;
				int lod = area->getLod();
				if (lod <= detailMaxLod)
				{
					area->loadInstanceData();
					area->getTerrain()->addRemoveDetails2(area);
				}
				else
				{
					area->unloadInstanceData();
					if (area->m_entity->getChilds().size() > 0)
						area->m_entity->removeAllChild(true);
					area->m_details.clear();
				}
			}, jobDetailDatas.data());

		updateDetailJob.addToQueue(Job::JobPriority::MEDIUM);
		updateDetailJob.waitCompletion(true);
		jobDetailDatas.clear();
#else
		for (auto& job : jobDetailDatas)
		{
			TerrainArea* area = job.m_area;
			int lod = area->getLod();
			if (lod <= detailMaxLod)
			{
				area->loadInstanceData();
				addRemoveDetails2(area);
			}
			else
			{
				area->unloadInstanceData();
				if (area->m_entity->getChilds().size() > 0)
					area->m_entity->removeAllChild(true);
				area->m_details.clear();
			}
		}
#endif
	}
}
void Terrain::addRemoveDetails(TerrainArea* area, int lod)
{
	for (const AreaDetails& terrainDetail : m_areaDetails)
	{
		if (lod < terrainDetail.m_lod)
		{
			// search if area has detail
			bool hasDetail = false;
			for (auto& areaDetail : area->m_details)
			{
				if (areaDetail.m_lod == terrainDetail.m_lod && areaDetail.m_name == terrainDetail.m_name)
				{
					hasDetail = true;
					break;
				}
			}
			if (hasDetail || terrainDetail.m_meshNames.size() == 0)
				continue;

			// create detail entities
			SCOPED_CPU_MARKER("add area detail");
			auto& areaDetail = area->m_details.emplace_back();
			areaDetail.m_name = terrainDetail.m_name;
			areaDetail.m_lod = terrainDetail.m_lod;
			for (int k = 0; k < terrainDetail.m_meshNames.size(); k++)
			{
				Entity* massentity = area->addDetailsInstance(
					terrainDetail.m_meshNames[k],
					terrainDetail.m_density,
					terrainDetail.m_allowedMaterials,
					terrainDetail.m_probability[k],
					terrainDetail.m_sizeRange,
					terrainDetail.m_normalWeight,
					terrainDetail.m_modelOffset[k]);

				TerrainDetailDrawableComponent* drawable = massentity->getComponent<TerrainDetailDrawableComponent>();
				drawable->setDoubleSidedFaces(terrainDetail.m_doubleSided);
				drawable->setColorTintGradient(terrainDetail.m_colorTint0, terrainDetail.m_colorTint1);
				drawable->setAlphaClipThs(terrainDetail.m_alphaCLipThs);
				//drawable->setMaxShadowCascade(terrainDetail.m_maxShadow);

				areaDetail.m_detailEntities.push_back(massentity);
			}
		}
		else
		{
			// search corresponding detail
			int index = -1;
			for (int k = 0; k < area->m_details.size(); k++)
			{
				auto& areaDetail = area->m_details[k];
				if (areaDetail.m_lod == terrainDetail.m_lod && areaDetail.m_name == terrainDetail.m_name)
				{
					index = k;
					break;
				}
			}
			if (index < 0)
				continue;

			// remove details
			SCOPED_CPU_MARKER("remove area detail");
			auto& areaDetail = area->m_details[index];
			for (Entity* massentity : areaDetail.m_detailEntities)
			{
				area->m_entity->removeChild(massentity, true);
			}
			int last = (int)area->m_details.size() - 1;
			if (index != last)
				area->m_details[index] = area->m_details[last];
			area->m_details.pop_back();
		}
	}
}

void Terrain::addRemoveDetails2(TerrainArea* area)
{
	int identifier = 0;
	for (const AreaDetails& terrainDetail : m_areaDetails)
	{
		if (area->getLod() <= terrainDetail.m_lod)
		{
			// search if area has detail
			bool hasDetail = false;
			for (auto& areaDetail : area->m_details)
			{
				if (areaDetail.m_lod == terrainDetail.m_lod && areaDetail.m_name == terrainDetail.m_name)
				{
					hasDetail = true;
					break;
				}
			}
			if (hasDetail || terrainDetail.m_meshNames.size() == 0)
				continue;

			// create detail entities
			SCOPED_CPU_MARKER("add area detail");
			auto& areaDetail = area->m_details.emplace_back();
			areaDetail.m_name = terrainDetail.m_name;
			areaDetail.m_lod = terrainDetail.m_lod;
			for (int k = 0; k < terrainDetail.m_meshResources.size(); k++)
			{
				Entity* massentity = area->addDetailsInstance(
					terrainDetail.m_meshResources[k],
					identifier + k,
					terrainDetail.m_normalWeight,
					terrainDetail.m_modelOffset[k]);

				TerrainDetailDrawableComponent* drawable = massentity->getComponent<TerrainDetailDrawableComponent>();
				drawable->setDoubleSidedFaces(terrainDetail.m_doubleSided);
				drawable->setColorTintGradient(terrainDetail.m_colorTint0, terrainDetail.m_colorTint1);
				drawable->setAlphaClipThs(terrainDetail.m_alphaCLipThs);
				//drawable->setMaxShadowCascade(terrainDetail.m_maxShadow);
				//for (auto it = terrainDetail.m_shaderTextureOverride.begin(); it != terrainDetail.m_shaderTextureOverride.end(); it++)
				//	drawable->setTextureOverride(it->first, it->second);

				areaDetail.m_detailEntities.push_back(massentity);
			}
		}
		else
		{
			// search corresponding detail
			int index = -1;
			for (int k = 0; k < area->m_details.size(); k++)
			{
				auto& areaDetail = area->m_details[k];
				if (areaDetail.m_lod == terrainDetail.m_lod && areaDetail.m_name == terrainDetail.m_name)
				{
					index = k;
					break;
				}
			}
			if (index < 0)
				continue;

			// remove details
			SCOPED_CPU_MARKER("remove area detail");
			auto& areaDetail = area->m_details[index];
			for (Entity* massentity : areaDetail.m_detailEntities)
			{
				area->m_entity->removeChild(massentity, true);
			}
			int last = (int)area->m_details.size() - 1;
			if (index != last)
				area->m_details[index] = area->m_details[last];
			area->m_details.pop_back();
		}
		identifier += (int)terrainDetail.m_meshResources.size();
	}
}
const std::vector<Terrain::AreaDetails>& Terrain::getAreaDetails()
{
	return m_areaDetails;
}
//

void Terrain::setWorld(World* _world)
{
	m_world = _world;
}

void Terrain::setVirtualTexture(TerrainVirtualTexture* virtualTexture)
{
	m_virtualTexture = virtualTexture;
}

TerrainVirtualTexture* Terrain::getVirtualTexture()
{
	return m_virtualTexture;
}

Material* Terrain::getDetailMaterial() const
{
	return m_terrainDetailMaterial;
}

std::string Terrain::getDirectory() const
{
	return m_directory;
}

const TerrainArea* Terrain::getArea(vec2i index) const
{
	vec2i gridindex = index - m_gridMinIndex;
	if (gridindex.x >= 0 && gridindex.x < m_gridSize.x && gridindex.y >= 0 && gridindex.y < m_gridSize.y)
		return m_grid[gridindex.x][gridindex.y];
	return nullptr;
}

const std::vector<float>& Terrain::getRadius() const
{
	return m_lodRadius;
}

// protected functions
void Terrain::recomputeGrid()
{
	// dele grid
	if (m_grid)
	{
		for (int i = 0; i < m_gridSize.x; i++)
			delete[] m_grid[i];
		delete[] m_grid;
		m_grid = nullptr;
	}

	m_gridSize = vec2i(0, 0);
	m_gridMinIndex = vec2i(std::numeric_limits<int>::max());

	// create a new one
	if (!m_areas.empty())
	{
		vec2i gridMax = -m_gridMinIndex;
		for (auto& it : m_areas)
		{
			m_gridMinIndex = vec2i::min(m_gridMinIndex, it.first.v);
			gridMax = vec2i::max(gridMax, it.first.v);
		}
		m_gridSize = gridMax - m_gridMinIndex + vec2i(1);

		m_grid = new TerrainArea**[m_gridSize.x];
		for (int i = 0; i < m_gridSize.x; i++)
		{
			m_grid[i] = new TerrainArea*[m_gridSize.y];
			for (int j = 0; j < m_gridSize.y; j++)
				m_grid[i][j] = nullptr;
		}

		for (auto& it : m_areas)
		{
			vec2i gridindex = it.first.v - m_gridMinIndex;
			it.second.m_gridIndex = gridindex;
			m_grid[gridindex.x][gridindex.y] = &it.second;

			/*int value = rand() % 4000;
			for (int mm = 0; mm < 3; mm++)
				for (int ee = 0; ee < 3; ee++)
					m_grid[gridindex.x][gridindex.y]->m_seeds[mm][ee] = value; */// 0.3 * gridindex.x + gridindex.y;
		}
	}

	// neigbouring seeds
	if (m_grid)
	{
		for (int i = 0; i < m_gridSize.x; i++)
			for (int j = 0; j < m_gridSize.y; j++)
			{
				TerrainArea* area = m_grid[i][j];
				if (!area)
					continue;

				area->m_center = vec4f((area->m_areaIndex.x + 0.5f) * 250.f, 0, (area->m_areaIndex.y + 0.5f) * 250.f, 1);
			}
	}
}
//

void Terrain::drawImGui(World& world)
{
#ifdef USE_IMGUI
	SCOPED_CPU_MARKER("Terrain");

	ImGui::Begin("Terrain");
	ImGui::PushID(this);
	const ImVec4 sectionColor = ImVec4(1, 1, 0, 1);
	static float areaSize = 17.f;
	static bool dragging = false;
	static ImVec2 dragMouseInit;
	static ImVec2 dragScrollInit;
	static bool centerOnPlayer = false;

	static std::string txt1 = "";
	//static std::string txt2 = "";
	//static std::string txt3 = "";

	ImGui::SliderFloat("Area size", &areaSize, 5.f, 100.f, "%.3f");
	ImGui::Checkbox("Center on player", &centerOnPlayer);
	ImGui::TextDisabled(txt1.c_str());
	//ImGui::TextDisabled(txt2.c_str());
	//ImGui::TextDisabled(txt3.c_str());

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const float availableSizeX = ImGui::GetContentRegionAvail().x;
	const float availableSizeY = ImGui::GetContentRegionAvail().y;
	ImGui::BeginChild("##TerrainPreview", ImVec2(availableSizeX, availableSizeY), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);

	window = ImGui::GetCurrentWindow();
	float frameSizeX = m_gridSize.x * areaSize;
	float frameSizeY = m_gridSize.y * areaSize;

	const ImVec2 cp = window->DC.CursorPos;
	const ImVec2 mouse = ImGui::GetIO().MousePos;
	ImVec2 windowMousePos = ImVec2(mouse.x - cp.x - ImGui::GetScrollX(), mouse.y - cp.y - ImGui::GetScrollY());
	
	ImDrawList* drawList = window->DrawList;
	const ImVec2 areaMargin = ImVec2(1, 1);
	const ImVec2 halfAreaSize = ImVec2(0.5f * areaSize, 0.5f * areaSize);
	ImVec2 playerPos = cp + ImVec2(m_currentPlayerPosInTile.x * areaSize, m_currentPlayerPosInTile.y * areaSize);

	//ImU32 diskColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 1.f, 1.f));
	//ImU32 ramColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 1.f, 0.4f, 1.f));

	bool tooltipEnabled = GImGui->HoveredWindow == window && 
		windowMousePos.x > 0 && windowMousePos.x < ImGui::GetContentRegionAvail().x && 
		windowMousePos.y > 0 && windowMousePos.y < ImGui::GetContentRegionAvail().y;

	if (centerOnPlayer)
	{
		dragging = false;
		ImVec2 delta = playerPos - cp - ImVec2(0.5f * availableSizeX, 0.5f * availableSizeY);
		ImGui::SetScrollX(delta.x);
		ImGui::SetScrollY(delta.y);
	}
	else
	{
		bool newDrag = tooltipEnabled && ImGui::IsMouseDown(ImGuiMouseButton_Left);
		if (newDrag && !dragging)
		{
			dragMouseInit = mouse;
			dragScrollInit = window->Scroll;
		}
		dragging = newDrag;
		if (dragging)
		{
			ImVec2 delta = mouse - dragMouseInit;
			ImGui::SetScrollX(dragScrollInit.x - delta.x);
			ImGui::SetScrollY(dragScrollInit.y - delta.y);
		}
	}

	//ImGui::RenderFrame(cp, cp + ImVec2(m_gridSize.x * areaSize, m_gridSize.y * areaSize), 0xFF404080, true, false);
	//ImGui::InvisibleButton("##canvas", ImVec2(m_gridSize.x * areaSize, m_gridSize.y * areaSize));

	SceneManager& scenemanager = m_world->getSceneManager();

	for (int i = 0; i < m_gridSize.x; i++)
		for (int j = 0; j < m_gridSize.y; j++)
		{
			TerrainArea* area = m_grid[i][j];
			if (!area)
				continue;

			ImVec2 areaCenter = cp + ImVec2(i * areaSize, j * areaSize) + halfAreaSize;
			ImVec2 m = areaCenter - halfAreaSize + areaMargin;
			ImVec2 M = areaCenter + halfAreaSize - areaMargin;
			drawList->AddRect(m, M, 0xFF404040, 0, ImDrawListFlags_None);
			bool inStreaming = (area->m_gridIndex.x >= m_currentStreamingMin.x && area->m_gridIndex.x <= m_currentStreamingMax.x &&
				area->m_gridIndex.y >= m_currentStreamingMin.y && area->m_gridIndex.y <= m_currentStreamingMax.y);

			ImU32 textColor = inStreaming ? 0xFF000000 : 0xFFFFFFFF;

			auto ComputeError = [&]()
			{
				if (!inStreaming)
					return false;
				if (!area->m_entity)
					return true;
				if (!scenemanager.isTracked(area->m_entity))
					return true;

				return false;
			};

			if (ComputeError())
				drawList->AddRectFilled(m + ImVec2(1, 1), M - ImVec2(1, 1), 0xFF404080);
			else if (inStreaming)
				drawList->AddRectFilled(m + ImVec2(1, 1), M - ImVec2(1, 1), 0xFF408040);
			
			std::string idx, idy;
			if (areaSize >= 50.f)
			{
				idx = "x:" + std::to_string(area->m_areaIndex.x);
				idy = "y:" + std::to_string(area->m_areaIndex.y);
			}
			else if (areaSize >= 40.f)
			{
				idx = std::to_string(area->m_areaIndex.x);
				idy = std::to_string(area->m_areaIndex.y);
			}

			if (areaSize >= 40.f)
			{
				ImVec2 txtsize = ImGui::CalcTextSize(idx.c_str());
				drawList->AddText(NULL, 14, areaCenter - ImVec2(0.5f * txtsize.x, +0.75f * txtsize.y), textColor, idx.c_str());
				txtsize = ImGui::CalcTextSize(idy.c_str());
				drawList->AddText(NULL, 14, areaCenter - ImVec2(0.5f * txtsize.x, -0.25f * txtsize.y), textColor, idy.c_str());
			}
			if (tooltipEnabled && mouse.x > m.x && mouse.x < M.x && mouse.y > m.y && mouse.y < M.y)
			{
				std::string fullName = "x:" + std::to_string(area->m_areaIndex.x) + " | y:" + std::to_string(area->m_areaIndex.y);

				ImGui::BeginTooltip();
				ImGui::Text(fullName.c_str());
				for (int k = 0; k < area->m_tiles.size(); k++)
				{
					const auto& tile = area->m_tiles[k];
					std::string tilestr = " * " + std::to_string(tile.m_lod) + " (" + std::to_string(tile.m_min.x) + ", " + std::to_string(tile.m_min.y) + ")";
					ImGui::Text(tilestr.c_str());
				}
				ImGui::EndTooltip();

				Debug::color = Debug::magenta;
				mat4f transform = mat4f::identity;
				float minHeight = area->getBoundingBox().min.y;
				float maxHeight = area->getBoundingBox().max.y;
				transform[3] = area->m_center + vec4f(0, 0.5f * (maxHeight + minHeight), 0, 0);
				Debug::drawCube(transform, vec4f(125.f, 0.5f * (maxHeight - minHeight), 125.f, 0));
				Debug::drawLineCube(mat4f::identity, area->m_center - vec4f(125.f, 500, 125.f, 0), area->m_center + vec4f(125.f, 500, 125.f, 0));
			}
		}

	drawList->AddCircleFilled(playerPos, 5, 0xFF202080);
	drawList->AddCircle(playerPos, 5, 0xFF2020FF);

	ImGui::Dummy(ImVec2(frameSizeX, frameSizeY));
	ImGui::EndChild();
	ImGui::PopID();
	ImGui::End();
#endif
}