#include "DrawableComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Resources/Shader.h>


DrawableComponent::DrawableComponent(const std::string& meshName, const std::string& shaderName)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
	m_shader = ResourceManager::getInstance()->getResource<Shader>(shaderName);
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

glm::vec3 DrawableComponent::getMeshBBMax() const
{
    GF_ASSERT(isValid());
	return (glm::vec3)m_mesh->getBoundingBox().max;
}

glm::vec3 DrawableComponent::getMeshBBMin() const
{
    GF_ASSERT(isValid());
	return (glm::vec3)m_mesh->getBoundingBox().min;
}
