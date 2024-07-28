#include "TerrainAreaDrawableComponent.h"
#include "Terrain.h"

#include <EntityComponent/Entity.hpp>
#include <Resources/Shader.h>
#include <Resources/ResourceManager.h>


TerrainAreaDrawableComponent::TerrainAreaDrawableComponent(TerrainArea* _area) : m_area(_area), m_waterShader(nullptr)
{

}
TerrainAreaDrawableComponent::~TerrainAreaDrawableComponent()
{
	ResourceManager::getInstance()->release(m_waterShader);
}

unsigned short TerrainAreaDrawableComponent::getInstanceDataSize() const
{
	return sizeof(TerrainAreaData) / sizeof(vec4f);
}
void TerrainAreaDrawableComponent::updateData(TerrainVirtualTexture::TextureTile& tile)
{
	vec4f position = getParentEntity()->getWorldPosition();
	m_data.position = vec2f(position.x, position.z);
	m_data.textureOffset = vec2f(tile.m_min.y, tile.m_min.x);
	m_data.morphingCase = 0;
}
void TerrainAreaDrawableComponent::pushInstanceData(Shader* _shader) const
{
	if (_shader)
	{
		int loc = _shader->getUniformLocation("instanceDataArray");
		if (loc)
			glUniform4fv(loc, sizeof(TerrainAreaData) / sizeof(vec4f), (const float*)&m_data);
	}
}
void TerrainAreaDrawableComponent::writeInstanceData(vec4f* _destination) const
{
	TerrainAreaData* data = (TerrainAreaData*)_destination;
	*data = m_data;
}


bool TerrainAreaDrawableComponent::hasConstantData() const
{
	return true;
}
void TerrainAreaDrawableComponent::pushConstantData(Shader* _shader) const
{
	if (_shader && m_area && m_area->getLod() >= 0)
	{
		TerrainConstantData constant;
		constant.lod = m_area->getLod();
		constant.textureSize = TerrainArea::g_lodPixelCount[m_area->getLod()];
		constant.heightAmplitude = TerrainArea::g_heightAmplitude / 65535.f;
		constant.seeLevel = TerrainArea::g_seeLevel;

		constant.morphDistance = m_area->getTerrain()->g_morphingRange;
		auto allRadius = m_area->getTerrain()->getRadius();
		for (int i = 0; i < allRadius.size(); i++)
			constant.morphRadius[i] = allRadius[i];

		int loc = _shader->getUniformLocation("constantData");
		if (loc >= 0)
			glUniform4fv(loc, sizeof(TerrainConstantData) / sizeof(vec4f), (const float*)&constant);
	}
}

void TerrainAreaDrawableComponent::pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass)
{
	// push terrain surface draw command
	Renderer::DrawElement element;
	element.shader = m_shader;
	element.mesh = m_mesh;
	element.batch = nullptr;
	element.entity = getParentEntity();

	uint64_t queue = m_shader->getRenderQueue();
	queue = queue << 48;
	uint32_t distance2 = distance;
	if (!isShadowPass && (queue & TransparentMask))
	{
		//compute 2's complement of d
		distance2 = ~distance2;
		distance2++;
	}
	if (isShadowPass)
		element.hash = distance2;
	else
		element.hash = queue | distance2;

	drawQueue.push_back(element);

	// push water draw
	if (!isShadowPass && hasWater() && getWaterShader())
	{
		element.shader = getWaterShader();
		distance2 = ~distance;
		distance2++; 
		queue = element.shader->getRenderQueue();
		queue = queue << 48;
		element.hash = queue | distance2;
		drawQueue.push_back(element);
	}
}

void TerrainAreaDrawableComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Drawable | (uint64_t)Entity::Flags::Fl_Terrain);
}

bool TerrainAreaDrawableComponent::hasWater() const
{
	return m_area && m_area->hasWater();
}
AxisAlignedBox TerrainAreaDrawableComponent::getBoundingBox() const
{
	if (m_area)
		return m_area->getBoundingBox();
	vec4f hs = vec4f(125.f, 100.f, 125.f, 0.f);
	return AxisAlignedBox(-hs, hs);
}
Shader* TerrainAreaDrawableComponent::getWaterShader() const
{
	return m_waterShader;
}
void  TerrainAreaDrawableComponent::setWaterShader(Shader* _shader)
{
	ResourceManager::getInstance()->release(m_shader);
	if (_shader) m_waterShader = ResourceManager::getInstance()->getResource<Shader>(_shader);
	else m_waterShader = nullptr;
}