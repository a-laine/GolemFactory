#include "EntityFactory.h"
#include "World/World.h"
#include "Instances/InstanceVirtual.h"
#include "Resources/ComponentResource.h"
#include "EntityComponent/AnimationEngine.h"


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

InstanceVirtual* EntityFactory::createDrawable(const std::string& meshName, const std::string& shaderName)
{
	InstanceVirtual* ins = new InstanceVirtual();
		ins->addComponent(new ComponentResource<Mesh>(ResourceManager::getInstance()->getMesh(meshName)));
		ins->addComponent(new ComponentResource<Shader>(ResourceManager::getInstance()->getShader(shaderName)));
	if(!ins || !world->manageObject(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}
	return ins;
}

/*InstanceVirtual* EntityFactory::createAnimatable(const std::string& meshName, const std::string& shaderName)
{
	InstanceVirtual* ins = new InstanceVirtual();
		ins->addComponent(new ComponentResource<Mesh>(ResourceManager::getInstance()->getMesh(meshName)));
		ins->addComponent(new ComponentResource<Shader>(ResourceManager::getInstance()->getShader(shaderName)));
	if(!ins || !world->manageObject(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}
	ins->computeCapsules();
	ins->initializeVBOVAO();
	return ins;
}*/

InstanceVirtual* EntityFactory::createAnimatable(const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName)
{
	InstanceVirtual* ins = new InstanceVirtual();
		ins->addComponent(new ComponentResource<Mesh>(ResourceManager::getInstance()->getMesh(meshName)));
		ins->addComponent(new ComponentResource<Shader>(ResourceManager::getInstance()->getShader(shaderName)));
		ins->addComponent(new ComponentResource<Skeleton>(ResourceManager::getInstance()->getSkeleton(skeletonName)));
		ins->addComponent(new ComponentResource<Animation>(ResourceManager::getInstance()->getAnimation(animationName)));
		ins->addComponent(new AnimationEngine(skeletonName, animationName));

	if(!ins || !world->manageObject(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}

	ins->getComponent<AnimationEngine>()->computeCapsules(ins->getComponent<ComponentResource<Mesh> >()->getResource());
	return ins;
}

