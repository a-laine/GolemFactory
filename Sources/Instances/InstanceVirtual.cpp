#include "InstanceVirtual.h"
#include "Resources/ComponentResource.h"
#include "EntityComponent/AnimationEngine.h"


//  Default
InstanceVirtual::InstanceVirtual() : 
	id(0), count(0), position(0.f, 0.f, 0.f), size(1.f, 1.f, 1.f), orientation(1.f), world(nullptr), modelMatrixNeedUpdate(true), model(1.f)
{}
InstanceVirtual::~InstanceVirtual() {}
//


//	Component related
void InstanceVirtual::addComponent(Component* component, ClassID type) { componentList.push_back({ component, type }); }
void InstanceVirtual::removeComponent(Component* component)
{
	for (unsigned int i = 0; i < componentList.size(); i++)
	{
		if (componentList[i].comp == component)
			componentList.erase(componentList.begin() + i);
	}
}


unsigned short InstanceVirtual::getNbComponents() const { return (unsigned short)componentList.size(); }
ClassID InstanceVirtual::getTypeID(unsigned short index) const { return (index < componentList.size()) ? componentList[index].type : Component::getStaticClassID(); }
Component* InstanceVirtual::getComponent(unsigned short index) { return (index < componentList.size()) ? componentList[index].comp : nullptr; }
const Component* InstanceVirtual::getComponent(unsigned short index) const { return (index < componentList.size()) ? componentList[index].comp : nullptr; }
Component* InstanceVirtual::getComponent(ClassID type)
{
	for (auto& elem : componentList)
	{
		if (elem.type == type)
			return elem.comp;
	}
	return nullptr;
}
const Component* InstanceVirtual::getComponent(ClassID type) const
{
	for (const auto& elem : componentList)
	{
		if (elem.type == type)
			return elem.comp;
	}
	return nullptr;
}
void InstanceVirtual::getAllComponents(ClassID type, std::vector<Component*>& components)
{
	for (auto& elem : componentList)
	{
		if (elem.type == type)
			components.push_back(elem.comp);
	}
}
void InstanceVirtual::getAllComponents(ClassID type, std::vector<const Component*>& components) const
{
	for (const auto& elem : componentList)
	{
		if (elem.type == type)
			components.push_back(elem.comp);
	}
}
//

//	Set/Get functions
void InstanceVirtual::setPosition(glm::vec3 p) { position = p; modelMatrixNeedUpdate = true; }
void InstanceVirtual::setSize(glm::vec3 s) { size = s; modelMatrixNeedUpdate = true; }
void InstanceVirtual::setOrientation(glm::mat4 m) { orientation = m; modelMatrixNeedUpdate = true; }
void InstanceVirtual::setParentWorld(World* parentWorld) { world = parentWorld; }

uint32_t InstanceVirtual::getId() const { return id; }
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
OrientedBox InstanceVirtual::getBoundingVolume()
{
	Mesh* m = getComponent<ComponentResource<Mesh> >()->getResource();
	if (m) return OrientedBox(glm::translate(glm::mat4(1.0), position), m->getBoundingBox().min*size, m->getBoundingBox().max*size);
	else return OrientedBox(glm::translate(glm::mat4(1.0), position));
}
World* InstanceVirtual::getParentWorld() const { return world; }
const InstanceVirtual::InstanceRenderingType InstanceVirtual::getRenderingType()
{
	ComponentResource<Shader>* shader = getComponent<ComponentResource<Shader> >();
	ComponentResource<Mesh>* mesh = getComponent<ComponentResource<Mesh> >();
	ComponentResource<Skeleton>* skeleton = getComponent<ComponentResource<Skeleton> >();
	AnimationEngine* animationEngine = getComponent<AnimationEngine>();

	if (shader && mesh && skeleton && animationEngine) return ANIMATABLE;
	else if (shader && mesh) return DRAWABLE;
	else return NONE;
}
const std::list<InstanceVirtual*>* InstanceVirtual::getChildList() const
{
	return nullptr;
}
//
