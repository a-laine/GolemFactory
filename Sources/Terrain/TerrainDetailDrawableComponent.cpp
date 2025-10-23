#include "TerrainDetailDrawableComponent.h"
#include "TerrainAreaDrawableComponent.h"
#include "Terrain.h"

#include <EntityComponent/Entity.hpp>
#include <Resources/Shader.h>
#include <Resources/Material.h>
#include <Resources/ResourceManager.h>
#include <Utiles/Debug.h>

TerrainDetailDrawableComponent::TerrainDetailDrawableComponent(TerrainArea* _area) : m_area(_area), m_vao(0), m_instancesBuffer(0)
{
	m_forceDoublesided = false;
	m_color0 = m_color1 = vec3f(1.f);
	m_alphaThs = -1.f;
}
TerrainDetailDrawableComponent::~TerrainDetailDrawableComponent()
{
	glDeleteBuffers(1, &m_instancesBuffer);
	glDeleteVertexArrays(1, &m_vao);

}

bool TerrainDetailDrawableComponent::hasConstantData() const
{
	return true;
}
void TerrainDetailDrawableComponent::pushConstantData(Shader* _shader) const
{
	if (_shader && m_area && m_area->getLod() >= 0)
	{
		TerrainDetailConstantData constant;
		int lod = m_area->getLod();
		constant.lod = lod;
		constant.textureSize = TerrainArea::g_lodPixelCount[lod];
		constant.heightAmplitude = TerrainArea::g_heightAmplitude / 65535.f;
		constant.seeLevel = TerrainArea::g_seeLevel;
		constant.position = vec2f(m_area->getCenter().x, m_area->getCenter().z);
		const TerrainVirtualTexture::TextureTile& tile = m_area->getTileData(lod);
		constant.textureOffset = vec2f(tile.m_min.y, tile.m_min.x);

		constant.morphDistance = m_area->getTerrain()->g_morphingRange;
		auto allRadius = m_area->getTerrain()->getRadius();
		for (int i = 0; i < allRadius.size(); i++)
			constant.morphRadius[i] = allRadius[i];

		constant.normalWorldWeight = m_normalWorldWeight;
		constant.fullModelScale = m_fullModelScale;
		constant.modelOffset = m_modelOffset;
		constant.color0 = m_color0;
		constant.color1 = m_color1;
		constant.alphaThs = m_alphaThs;

		int loc = _shader->getUniformLocation("constantData");
		if (loc >= 0)
			glUniform4fv(loc, sizeof(TerrainDetailConstantData) / sizeof(vec4f), (const float*)&constant);
	}
}
void TerrainDetailDrawableComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Drawable |(uint64_t)Entity::Flags::Fl_Terrain | (uint64_t)Entity::Flags::Fl_MassInstancing);
}

void TerrainDetailDrawableComponent::pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass)
{
#ifdef USE_IMGUI
	if (!m_area->getTerrain()->getAreaDetails()[m_detailIndex].m_visible)
		return;
#endif

	Renderer::DrawElement element;
	element.material = m_material;
	element.mesh = m_mesh;
	element.batch = nullptr;
	element.entity = getParentEntity();

	uint64_t queue = m_material->getShader()->getRenderQueue();
	queue = queue << 48;
	if (m_forceDoublesided)
		queue &= ~FaceCullingMask;
	if (!isShadowPass && (queue & TransparentMask))
	{
		//compute 2's complement of d
		distance = ~distance;
		distance++;
	}
	if (m_alphaThs > 0.f)
		queue |= TransparentMask;

	if (isShadowPass)
		element.hash = distance;
	else
		element.hash = queue | distance;

	drawQueue.push_back(element);
}

bool TerrainDetailDrawableComponent::hasCustomDraw() const
{
	return true;
}

void TerrainDetailDrawableComponent::customDraw(Renderer* _renderer, unsigned int& _instanceDrawnCounter, unsigned int& _drawCallsCounter, unsigned int& _trianglesDrawnCounter) const
{
	_renderer->loadVAO(m_vao);
	const unsigned int count = m_instancesDatas.size();
	glDrawElementsInstanced(GL_TRIANGLES, m_mesh->getNumberIndices(), m_mesh->getIndicesType(), NULL, count);

	_instanceDrawnCounter += count;
	_drawCallsCounter++;
	_trianglesDrawnCounter += count * m_mesh->getNumberFaces();
}


AxisAlignedBox TerrainDetailDrawableComponent::getBoundingBox() const
{
	float r = m_mesh->getBoundingBox().toSphere().radius;
	AxisAlignedBox box;
	if (m_area)
	{
		box = m_area->getBoundingBox();
		box.min -= vec4f(r, r, r, 0);
		box.max += vec4f(r, r, r, 0);
	}
	else
	{
		vec4f hs = vec4f(125.f + r, 100.f + r, 125.f + r, 0.f);
		box = AxisAlignedBox(-hs, hs);
	}
	return box;
}

void TerrainDetailDrawableComponent::setInstanceData(std::vector<vec4ui>& data)
{
	m_instancesDatas.swap(data);

	if (glIsBuffer(m_instancesBuffer))
	{

	}
}

void TerrainDetailDrawableComponent::setFullModelScale(float scale)
{
	m_fullModelScale = scale;
}
float TerrainDetailDrawableComponent::getFullModelScale() const
{
	return m_fullModelScale;
}
void TerrainDetailDrawableComponent::setWorldNormalWeight(float weight)
{
	m_normalWorldWeight = weight;
}
void TerrainDetailDrawableComponent::setModelOffset(float offset)
{
	m_modelOffset = offset;
}
void TerrainDetailDrawableComponent::setDoubleSidedFaces(bool enable)
{
	m_forceDoublesided = enable;
}
void TerrainDetailDrawableComponent::setColorTintGradient(vec3f c0, vec3f c1)
{
	m_color0 = c0;
	m_color1 = c1;
}
void TerrainDetailDrawableComponent::setAlphaClipThs(float ths)
{
	m_alphaThs = ths;
}
void TerrainDetailDrawableComponent::setDetailIndex(uint16_t index) 
{
	m_detailIndex = index;
}

void TerrainDetailDrawableComponent::initializeVBO()
{
	glGenBuffers(1, &m_instancesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_instancesDatas.size() * sizeof(vec4ui), m_instancesDatas.data(), GL_STATIC_DRAW);
}

void TerrainDetailDrawableComponent::initializeVAO()
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_mesh->verticesBuffer);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	if (!m_mesh->normals.empty())
	{
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, m_mesh->normalsBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if (!m_mesh->uvs.empty())
	{
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, m_mesh->uvsBuffer);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	// instance datas
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, 0, NULL);
	glVertexAttribDivisor(3, 1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh->facesBuffer);
	glBindVertexArray(0);
}

void TerrainDetailDrawableComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(1, 0.5, 0, 1);
	std::ostringstream unicName;
	unicName << "Terrain detail component##" << (uintptr_t)this;

	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Mesh");
		ImGui::Indent();
		ImGui::Text("name : %s", m_mesh->name.c_str());
		ImGui::Text("vertices count : %d", m_mesh->getNumberVertices());
		ImGui::Text("faces count : %d", m_mesh->getNumberFaces());
		vec4f center = 0.5f * (m_mesh->getBoundingBox().max + m_mesh->getBoundingBox().min);
		vec4f size = 0.5f * (m_mesh->getBoundingBox().max - m_mesh->getBoundingBox().min);
		ImGui::Text("local aabb center : %.2f, %.2f, %.2f", center.x, center.y, center.z);
		ImGui::Text("local aabb size : %.2f, %.2f, %.2f", size.x, size.y, size.z);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Material");
		ImGui::Indent();
		ImGui::Text("name : %s", m_material->name.c_str());

		const auto& textures = m_material->getTextures();
		const auto& shaderTextures = m_material->getShader()->getTextures();
		if (!textures.empty())
		{
			ImGui::TextColored(ImVec4(0.7f, 1.f, 0.7f, 1.f), "Binded textures :");
			ImGui::Indent();
			for (int i = 0; i < shaderTextures.size(); i++)
			{
				if (shaderTextures[i].isGlobalAttribute)
					ImGui::Text("Global : %s", shaderTextures[i].defaultResource.c_str());
				else if (textures[i])
					ImGui::Text("Location %d : %s", i, textures[i]->name.c_str());
			}
			ImGui::Unindent();
		}
		const auto& uniforms = m_material->getShader()->getUniforms();
		if (!uniforms.empty())
		{
			ImGui::TextColored(ImVec4(0.7f, 1.f, 0.7f, 1.f), "Parameters :");
			ImGui::Indent();
			for (auto it = uniforms.begin(); it != uniforms.end(); it++)
			{
				ImGui::Text("%s : %s", it->first.c_str(), it->second.c_str());
			}
			ImGui::Unindent();
		}
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Instances");
		ImGui::Indent();
		ImGui::Text("instance count : %d", m_instancesDatas.size());
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Constant data");
		ImGui::Indent();
		ImGui::SliderFloat("NormalWorldWeight", &m_normalWorldWeight, 0.f, 1.f);
		ImGui::SliderFloat("FullModelScale", &m_fullModelScale, 0.01f, 30.f);
		ImGui::ColorEdit3("Tint 0", &m_color0.x);
		ImGui::ColorEdit3("Tint 1", &m_color1.x);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw mesh bounding box", &m_drawMeshBoundingBox);
		ImGui::Checkbox("Visible", &m_visible);
		ImGui::Unindent();

		ImGui::TreePop();
	}


	if (m_drawMeshBoundingBox)
	{
		Debug::color = vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w);
		const auto& aabb = m_mesh->getBoundingBox();
		Debug::drawLineCube(getParentEntity()->getWorldTransformMatrix(), aabb.min, aabb.max);
	}
#endif // USE_IMGUI
}