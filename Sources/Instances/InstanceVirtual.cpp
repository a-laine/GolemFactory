#include "InstanceVirtual.h"

//  Default
InstanceVirtual::InstanceVirtual(InstanceType instanceType) : 
	type(instanceType), id(0), count(0), position(0.f, 0.f, 0.f), size(1.f, 1.f, 1.f), orientation(1.f), world(nullptr), modelMatrixNeedUpdate(true), model(1.f)
{}
InstanceVirtual::~InstanceVirtual() {}
//


//	Set/Get functions
void InstanceVirtual::setPosition(glm::vec3 p) { position = p; modelMatrixNeedUpdate = true; }
void InstanceVirtual::setSize(glm::vec3 s) { size = s; modelMatrixNeedUpdate = true; }
void InstanceVirtual::setOrientation(glm::mat4 m) { orientation = m; modelMatrixNeedUpdate = true; }

void InstanceVirtual::setParentWorld(World* parentWorld) { world = parentWorld; }


glm::vec3 InstanceVirtual::getPosition() const { return position; }
glm::vec3 InstanceVirtual::getSize() const  { return size; }
glm::mat4 InstanceVirtual::getOrientation() const { return orientation; }
glm::mat4 InstanceVirtual::getModelMatrix()
{
	if (!modelMatrixNeedUpdate) return model;
	else
	{
		modelMatrixNeedUpdate = false;
		model = glm::translate(glm::mat4(1.0), position);
		model = model * orientation;
		model = glm::scale(model, size);
		return model;
	}
}

World* InstanceVirtual::getParentWorld() const { return world; }

InstanceVirtual::InstanceType InstanceVirtual::getType() const { return type; }
uint32_t InstanceVirtual::getId() const { return id; }
glm::vec3 InstanceVirtual::getBBMax() const { return glm::vec3(0.f, 0.f, 0.f); }
glm::vec3 InstanceVirtual::getBBMin() const { return glm::vec3(0.f, 0.f, 0.f); }
float InstanceVirtual::getBSRadius() const
{
	return std::max(glm::length(getBBMax()),glm::length(getBBMin()));
};


Shader* InstanceVirtual::getShader() const { return nullptr; }
Animation* InstanceVirtual::getAnimation() const { return nullptr; }
Skeleton* InstanceVirtual::getSkeleton() const { return nullptr; }
Mesh* InstanceVirtual::getMesh() const { return nullptr; }
std::vector<glm::mat4> InstanceVirtual::getPose() const { return std::vector<glm::mat4>(); }
const std::list<InstanceVirtual*>* InstanceVirtual::getChildList() const { return nullptr; }
//
