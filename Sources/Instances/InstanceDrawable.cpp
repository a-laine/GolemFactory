#include "InstanceDrawable.h"


//  Default
InstanceDrawable::InstanceDrawable(std::string meshName) : InstanceVirtual()
{
	mesh = ResourceManager::getInstance()->getMesh(meshName);
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
void InstanceDrawable::draw(Shader* shaderToUse)
{
	if (!shaderToUse || !mesh) return;

	glm::mat4 model(1.0);
	model = glm::translate(model,position);
	model = model * rotationMatrix;
	model = glm::scale(model, size);
	
	shaderToUse->loadUniformMatrix('m',&model[0][0]);
	mesh->draw();
}

void InstanceDrawable::setOrientation(glm::mat4 m)
{
	rotationMatrix = m;
}
//
