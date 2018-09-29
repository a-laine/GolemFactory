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

glm::vec3 DrawableComponent::getMeshBBMax() const
{
	return m_mesh->getBoundingBox().max;
}

glm::vec3 DrawableComponent::getMeshBBMin() const
{
	return m_mesh->getBoundingBox().min;
}
