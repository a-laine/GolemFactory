#include "EntityFactory.h"
#include "Resources/ResourceManager.h"
#include "Utiles/IncrementalHull.h"
#include "Resources/Loader/MeshSaver.h"

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
		createDrawable(object, "icosphere.obj", "default", false);
		object->setShape(new Sphere(glm::vec3(0.f), 1.f));
	}
	else if (type == "cube")
	{
		createDrawable(object, "cube2.obj", "wired", false);
		DrawableComponent* drawable = object->getComponent<DrawableComponent>();
		object->setShape(new OrientedBox(glm::mat4(1.f), drawable->getMeshBBMin(), drawable->getMeshBBMax()));
	}
	else if(type == "tree")
		createDrawable(object, "firTree1.obj", "default", true);
	else if(type == "rock")
		createDrawable(object, "rock1.obj", "default", true);
	else { GF_ASSERT(0); }
	return object;
}

void EntityFactory::addToScene(Entity* object)
{
	world->addToScene(object);
}

void EntityFactory::createDrawable(Entity* object, const std::string& meshName, const std::string& shaderName, const bool& hullGeneration)
{
	DrawableComponent* drawable = new DrawableComponent(meshName, shaderName);
	object->addComponent(drawable);

	//	link, load or generate a convex hull for every entities
	if (hullGeneration)
	{
		std::string hullname = meshName;
		if (hullname.find_last_of('/') != std::string::npos)
			hullname = hullname.substr(hullname.find_last_of('/') + 1);
		if (hullname.find_first_of('.') != std::string::npos)
			hullname = hullname.substr(0, hullname.find_first_of('.'));
		hullname = "Hull/hull_" + hullname + Mesh::extension;

		Mesh* m = ResourceManager::getInstance()->findResource<Mesh>(hullname);
		if (m)
		{
			object->setShape(new Hull(m));
		}
		else if (ResourceManager::getInstance()->loadableResource<Mesh>(hullname))
		{
			m = ResourceManager::getInstance()->getResource<Mesh>(hullname);
			ToolBox::optimizeHullMesh(m);
			object->setShape(new Hull(m));
		}
		else
		{
			std::cout << "EntityFactory : Fail found hull of name : " << hullname << ". It will be automatically generated."<< std::endl;
			IncrementalHull hullgenerator;
			m = hullgenerator.getConvexHull(drawable->getMesh());
			m->name = hullname;
			MeshSaver::save(m, ResourceManager::getInstance()->getRepository() + "Meshes/", hullname);

			ToolBox::optimizeHullMesh(m);
			ResourceManager::getInstance()->addResource(m);
			object->setShape(new Hull(m));
		}
		ResourceManager::getInstance()->release(m);
	}
	/*else
	{
		object->setShape(new OrientedBox(glm::mat4(1.f), drawable->getMeshBBMin(), drawable->getMeshBBMax()));
	}*/
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

