#include "InstanceVirtual.h"

//  Default
InstanceVirtual::InstanceVirtual(InstanceType instanceType) : type(instanceType), id(0), count(0), position(0.f, 0.f, 0.f), size(1.f, 1.f, 1.f), orientation(1.f) {}
InstanceVirtual::~InstanceVirtual() {}
//


//	Set/Get functions
InstanceVirtual::InstanceType InstanceVirtual::getType() const { return type; }
void InstanceVirtual::setPosition(glm::vec3 p) { position = p; }
void InstanceVirtual::setSize(glm::vec3 s) { size = s; }
void InstanceVirtual::setOrientation(glm::mat4 m) { orientation = m; }

glm::vec3 InstanceVirtual::getPosition() const{ return position; }
glm::vec3 InstanceVirtual::getSize() const  { return size; }
glm::mat4 InstanceVirtual::getOrientation() const { return orientation; }

glm::mat4 InstanceVirtual::getModelMatrix() const
{
	glm::mat4 model(1.0);
	model = glm::translate(model, position);
	model = model * orientation;
	model = glm::scale(model, size);
	return model;
}
glm::vec3 InstanceVirtual::getBBSize() { return glm::vec3(0.f, 0.f, 0.f); }
float InstanceVirtual::getBSRadius() { return 0.f; };

Shader* InstanceVirtual::getShader() const { return nullptr; }
Animation* InstanceVirtual::getAnimation() const { return nullptr; }
Skeleton* InstanceVirtual::getSkeleton() const { return nullptr; }
Mesh* InstanceVirtual::getMesh() const { return nullptr; }
const std::list<InstanceVirtual*>& InstanceVirtual::getChildList() const
{
	std::list<InstanceVirtual*> l;
	return l;
}
//
