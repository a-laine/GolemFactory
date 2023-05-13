#include "DrawableComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Resources/Shader.h>
#include <Utiles/Debug.h>


DrawableComponent::DrawableComponent(const std::string& meshName, const std::string& shaderName)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
	m_shader = ResourceManager::getInstance()->getResource<Shader>(shaderName);
}

DrawableComponent::DrawableComponent(const DrawableComponent* other)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(other->m_mesh->name);
	m_shader = ResourceManager::getInstance()->getResource<Shader>(other->m_shader->name);
}

DrawableComponent::~DrawableComponent()
{
	ResourceManager::getInstance()->release(m_mesh);
	ResourceManager::getInstance()->release(m_shader);
}

void DrawableComponent::setShader(const std::string& shaderName)
{
	ResourceManager::getInstance()->release(m_shader);
	m_shader = ResourceManager::getInstance()->getResource<Shader>(shaderName);
}

void DrawableComponent::setShader(Shader* shader)
{
	ResourceManager::getInstance()->release(m_shader);
	if(shader) m_shader = ResourceManager::getInstance()->getResource<Shader>(shader);
	else m_shader = nullptr;
}

void DrawableComponent::setMesh(const std::string& meshName)
{
	ResourceManager::getInstance()->release(m_mesh);
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
}

void DrawableComponent::setMesh(Mesh* mesh)
{
	ResourceManager::getInstance()->release(m_mesh);
	if(mesh) m_mesh = ResourceManager::getInstance()->getResource<Mesh>(mesh);
	else m_mesh = nullptr;
}

Shader* DrawableComponent::getShader() const
{
	return m_shader;
}

Mesh* DrawableComponent::getMesh() const
{
	return m_mesh;
}

bool DrawableComponent::isValid() const
{
    return m_mesh && m_mesh->isValid() && m_shader && m_shader->isValid();
}

bool DrawableComponent::hasSkeleton() const
{
    GF_ASSERT(isValid());
    return m_mesh->hasSkeleton();
}

vec4f DrawableComponent::getMeshBBMax() const
{
    GF_ASSERT(isValid());
	return m_mesh->getBoundingBox().max;
}

vec4f DrawableComponent::getMeshBBMin() const
{
    GF_ASSERT(isValid());
	return m_mesh->getBoundingBox().min;
}

void DrawableComponent::onAddToEntity(Entity* entity)
{
	entity->setFlags((uint64_t)Entity::Flags::Fl_Drawable);
}

void DrawableComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(1, 0.5, 0, 1);
	std::ostringstream unicName;
	unicName << "Drawable component##" << (uintptr_t)this;

	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Mesh");
		ImGui::Indent();
		ImGui::Text("name : %s", m_mesh->name.c_str());
		ImGui::Text("vertices count : %d", m_mesh->getNumberVertices());
		ImGui::Text("faces count : %d", m_mesh->getNumberFaces());
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Shader");
		ImGui::Indent();
		ImGui::Text("name : %s", m_shader->name.c_str());
		
		const auto& textures = m_shader->getTextures();
		if (!textures.empty())
		{
			ImGui::TextColored(ImVec4(0.7, 1, 0.7, 1), "Binded textures :");
			ImGui::Indent();
			for (int i = 0; i < textures.size(); i++)
			{
				ImGui::Text("Location %d : %s", i, textures[i]->name.c_str());
			}
			ImGui::Unindent();
		}
		const auto& uniforms = m_shader->getUniforms();
		if (!uniforms.empty())
		{
			ImGui::TextColored(ImVec4(0.7, 1, 0.7, 1), "Parameters :");
			ImGui::Indent();
			for (auto it = uniforms.begin(); it != uniforms.end(); it++)
			{
				ImGui::Text("%s : %s", it->first.c_str(), it->second.c_str());
			}
			ImGui::Unindent();
		}
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