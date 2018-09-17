#include "InstanceDrawable.h"


//  Default
InstanceDrawable::InstanceDrawable(const std::string& meshName, const std::string& shaderName) : InstanceVirtual(InstanceVirtual::DRAWABLE)
{
	mesh = ResourceManager::getInstance()->getMesh(meshName);
	shader = ResourceManager::getInstance()->getShader(shaderName);
}
InstanceDrawable::~InstanceDrawable()
{
	ResourceManager::getInstance()->release(mesh);
	ResourceManager::getInstance()->release(shader);
}
//

//	Set/get functions
void InstanceDrawable::setShader(std::string shaderName)
{
	ResourceManager::getInstance()->release(shader);
	shader = ResourceManager::getInstance()->getShader(shaderName);
}
void InstanceDrawable::setShader(Shader* s)
{
	ResourceManager::getInstance()->release(shader);
	if (s) shader = ResourceManager::getInstance()->getShader(s->name);
	else shader = nullptr;
}
void InstanceDrawable::setMesh(std::string meshName)
{
	ResourceManager::getInstance()->release(mesh);
	mesh = ResourceManager::getInstance()->getMesh(meshName);
}
void InstanceDrawable::setMesh(Mesh* m)
{
	ResourceManager::getInstance()->release(mesh);
	if (m) mesh = ResourceManager::getInstance()->getMesh(m->name);
	else mesh = nullptr;
}

glm::vec3 InstanceDrawable::getBBMax() const  { return mesh->aabb_max; }
glm::vec3 InstanceDrawable::getBBMin() const  { return mesh->aabb_min; }
float InstanceDrawable::getBSRadius() const
{
	float x = std::max(abs(mesh->aabb_min.x), abs(mesh->aabb_max.x));
	float y = std::max(abs(mesh->aabb_min.y), abs(mesh->aabb_max.y));
	float z = std::max(abs(mesh->aabb_min.z), abs(mesh->aabb_max.z));
	return glm::length(glm::vec3(x,y,z));
}
Shader* InstanceDrawable::getShader() const { return shader; }
Mesh* InstanceDrawable::getMesh() const { return mesh; }
//