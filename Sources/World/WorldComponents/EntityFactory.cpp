#include "EntityFactory.h"

#include <World/World.h>
#include <Instances/InstanceVirtual.h>
#include <Instances/InstanceDrawable.h>
#include <Instances/InstanceContainer.h>
#include <Instances/InstanceAnimatable.h>



EntityFactory::EntityFactory(World* parentWorld)
	: world(parentWorld)
{}

InstanceVirtual* EntityFactory::createObject(const std::string& type, const glm::vec3& position, const glm::vec3& scale)
{
	InstanceVirtual* object = createByType(type);
	if(object)
	{
		object->setPosition(position);
		object->setSize(scale);
		addToScene(object);
	}
	return object;
}

InstanceVirtual* EntityFactory::createObject(const std::string& type, const glm::vec3& position, const glm::vec3& scale, const glm::mat4& orientation)
{
	InstanceVirtual* object = createByType(type);
	if(object)
	{
		object->setPosition(position);
		object->setSize(scale);
		object->setOrientation(orientation);
		addToScene(object);
	}
	return object;
}

InstanceVirtual* EntityFactory::createByType(const std::string& type)
{
	if(type == "peasent")
		return createAnimatable("peasant", "human", "simple_peasant", "skinning");
	else if(type == "sphere")
		return createDrawable("icosphere.obj", "default");
	else if(type == "cube")
		return createDrawable("default", "wired");
	else if(type == "tree")
		return createDrawable("firTree1.obj", "default");
	else if(type == "rock")
		return createDrawable("rock1.obj", "default");
	return nullptr;
}

void EntityFactory::addToScene(InstanceVirtual* object)
{
	world->getSceneManager().addObject(object);
}

InstanceDrawable* EntityFactory::createDrawable(const std::string& meshName, const std::string& shaderName)
{
	InstanceDrawable* ins = new InstanceDrawable(meshName, shaderName);
	if(!ins || !world->manageObject(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}
	return ins;
}

InstanceAnimatable* EntityFactory::createAnimatable(const std::string& meshName, const std::string& shaderName)
{
	InstanceAnimatable* ins = new InstanceAnimatable(meshName, shaderName);
	if(!ins || !world->manageObject(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}
	ins->computeCapsules();
	ins->initializeVBOVAO();
	return ins;
}

InstanceAnimatable* EntityFactory::createAnimatable(const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName)
{
	InstanceAnimatable* ins = new InstanceAnimatable(meshName, shaderName);
	if(!ins || !world->manageObject(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}
	ins->setSkeleton(skeletonName);
	ins->setAnimation(animationName);
	ins->computeCapsules();
	ins->initializeVBOVAO();
	return ins;
}

/*InstanceContainer* EntityFactory::createContainer()
{
	InstanceContainer* ins = new InstanceContainer();
	if (!ins || !world->manageObject(ins))
	{
		if (ins) delete ins;
		return nullptr;
	}
	return ins;
}*/

