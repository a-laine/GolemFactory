#include "InstanceDrawable.h"


//  Default
InstanceDrawable::InstanceDrawable(std::string meshName, std::string shaderName) : InstanceVirtual()
{
	mesh = ResourceManager::getInstance()->getMesh(meshName);
	shader = ResourceManager::getInstance()->getShader(shaderName);
	if (!mesh) return;

	bbsize.x = mesh->sizeX.y - mesh->sizeX.x;
	bbsize.y = mesh->sizeY.y - mesh->sizeY.x;
	bbsize.z = mesh->sizeZ.y - mesh->sizeZ.x;
}
InstanceDrawable::~InstanceDrawable()
{
	ResourceManager::getInstance()->release(mesh);
}
//

//	Public functions
void InstanceDrawable::setOrientation(glm::mat4 m)
{
	rotationMatrix = m;
}
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


Shader* InstanceDrawable::getShader() const
{
	return shader;
}
Mesh* InstanceDrawable::getMesh() const
{
	return mesh;
}
glm::mat4 InstanceDrawable::getModelMatrix() const
{
	glm::mat4 model(1.0);
		model = glm::translate(model, position);
		model = model * rotationMatrix;
		model = glm::scale(model, size);
	return model;
}
//
