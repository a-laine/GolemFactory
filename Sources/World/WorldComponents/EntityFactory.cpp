#include "EntityFactory.h"
#include "Resources/ResourceManager.h"
#include "Utiles/IncrementalHull.h"

#include <Utiles/Assert.hpp>
#include <World/World.h>
#include <EntityComponent/Entity.hpp>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Animation/AnimationComponent.h>



EntityFactory::EntityFactory(World* parentWorld)
	: world(parentWorld)
{}

Entity* EntityFactory::createObject(const std::string& type, const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation)
{
	Entity* object = createByType(type);
	if(object)
	{
		object->setTransformation(position, scale, orientation);
		addToScene(object);
	}
	return object;
}

Entity* EntityFactory::createObject(const std::vector<Component*>& components, const glm::vec3& position, const glm::vec3& scale, const glm::quat& orientation)
{
	Entity* object = createEntity();
	addComponents(object, components);
	object->setTransformation(position, scale, orientation);
	addToScene(object);
	return object;
}

Entity* EntityFactory::createEntity()
{
	return world->getNewEntity();
}

Entity* EntityFactory::createByType(const std::string& type)
{
	Entity* object = createEntity();
	if(type == "peasant")
	{
		createAnimatable(object, "peasant", "human", "simple_peasant", "skinning");
		object->setShape(new Capsule(glm::vec3(0.f, 0.f, -3.f), glm::vec3(0.f, 0.f, 2.4f), 1.7f));
	}
	else if(type == "sphere")
	{
		createDrawable(object, "icosphere.obj", "default");
		object->setShape(new Sphere(glm::vec3(0.f), 1.f));
	}
	else if(type == "cube")
		createDrawable(object, "cube2.obj", "wired");
	else if(type == "tree")
		createDrawable(object, "firTree1.obj", "default");
	else if(type == "rock")
		createDrawable(object, "rock1.obj", "default");
	else { GF_ASSERT(0); }
	return object;
}

void EntityFactory::addToScene(Entity* object)
{
	world->addToScene(object);
}

void EntityFactory::createDrawable(Entity* object, const std::string& meshName, const std::string& shaderName)
{
	DrawableComponent* drawable = new DrawableComponent(meshName, shaderName);
	object->addComponent(drawable);
	Mesh* m = ResourceManager::getInstance()->findResource<Mesh>("hull_" + meshName);
	if (m)
		object->setShape(new Hull(m));
	else
	{
		std::cout << "fail found hull of name : " << "hull_" + meshName << std::endl;
		IncrementalHull hullgenerator;
		m = hullgenerator.getConvexHull(drawable->getMesh());
		ResourceManager::getInstance()->addResource(m);
		std::cout << "  added with name : " << m->name << std::endl;
		object->setShape(new Hull(m));
	}

	//drawable->setMesh(m);

    //object->setShape(new OrientedBox(glm::mat4(1.f), drawable->getMeshBBMin(), drawable->getMeshBBMax()));
}

void EntityFactory::createAnimatable(Entity* object, const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName)
{
	DrawableComponent* drawable = new DrawableComponent(meshName, shaderName);
	SkeletonComponent* skeleton = new SkeletonComponent(skeletonName);
	AnimationComponent* animation = new AnimationComponent(animationName);
	skeleton->computeCapsules(drawable->getMesh());
	skeleton->initializeVBOVAO();
	object->addComponent(drawable);
	object->addComponent(skeleton);
	object->addComponent(animation);
	object->setShape(new Sphere(glm::vec3(0.f), 0.5f * glm::length(drawable->getMeshBBMax() - drawable->getMeshBBMin())));
}

void EntityFactory::addComponents(Entity* object, const std::vector<Component*>& components)
{
	for(Component* comp : components)
	{
		object->addComponent(comp, comp->getClassID());
	}
}

