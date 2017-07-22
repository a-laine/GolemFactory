#include "InstanceDrawable.h"


//  Default
InstanceDrawable::InstanceDrawable(std::string meshName, std::string shaderName) : InstanceVirtual(InstanceVirtual::DRAWABLE)
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


glm::vec3 InstanceDrawable::getBBSize() const
{
	if (!mesh) return glm::vec3(0.f, 0.f, 0.f);
	else return glm::vec3(
		size.x*(mesh->sizeX.y - mesh->sizeX.x),
		size.y*(mesh->sizeY.y - mesh->sizeY.x),
		size.z*(mesh->sizeZ.y - mesh->sizeZ.x)   );
}
float InstanceDrawable::getBSRadius() const
{
	return std::sqrtf(	(mesh->sizeX.y - mesh->sizeX.x)*(mesh->sizeX.y - mesh->sizeX.x) +
						(mesh->sizeY.y - mesh->sizeY.x)*(mesh->sizeY.y - mesh->sizeY.x) +
						(mesh->sizeZ.y - mesh->sizeZ.x)*(mesh->sizeZ.y - mesh->sizeZ.x)    ) * 0.5f;
}
Shader* InstanceDrawable::getShader() const { return shader; }
Mesh* InstanceDrawable::getMesh() const { return mesh; }
//