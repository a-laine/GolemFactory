#include "EntityFactory.h"
#include <Resources/ResourceManager.h>
#include <Utiles/IncrementalHull.h>
#include <Utiles/Parser/Reader.h>
#include <Resources/Loader/MeshSaver.h>

#include <Utiles/Assert.hpp>
#include <World/World.h>
#include <EntityComponent/Entity.hpp>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Animation/AnimationComponent.h>
#include <Physics/Shapes/Collider.h>



EntityFactory::EntityFactory(World* parentWorld)
	: world(parentWorld)
{}

Entity* EntityFactory::createObject(const std::string& type, const vec4f& position, const float& scale, const quatf& orientation, const std::string& name)
{
	Entity* object = createByType(type);
	if(object)
	{
		object->setName(name);
		object->setWorldTransformation(position, scale, orientation);
		addToScene(object);
	}
	return object;
}

Entity* EntityFactory::createObject(const std::vector<Component*>& components, const vec4f& position, const float& scale, const quatf& orientation, const std::string& name)
{
	Entity* object = createEntity();
	addComponents(object, components);
	object->setName(name);
	object->setWorldTransformation(position, scale, orientation);
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

		Collider* collider = new Collider(new Capsule(vec4f(0.f, 0.f, -3.f, 1), vec4f(0.f, 0.f, 2.4f, 1), 1.7f));
		object->addComponent(collider);
		object->recomputeBoundingBox();
	}
	else if(type == "sphere")
	{
		createDrawable(object, "icosphere.obj", "default", false);

		Collider* collider = new Collider(new Sphere(vec4f(0.f), 1.f));
		object->addComponent(collider);
		object->recomputeBoundingBox();
	}
	else if (type == "cube")
	{
		createDrawable(object, "cube2.obj", "default", false);
		DrawableComponent* drawable = object->getComponent<DrawableComponent>();

		Collider* collider = new Collider(new OrientedBox(mat4f::identity, drawable->getMeshBBMin(), drawable->getMeshBBMax()));
		object->addComponent(collider);
		object->recomputeBoundingBox();
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
	RigidBody* rigidbody = object->getComponent<RigidBody>();
	if (rigidbody)
		rigidbody->initialize(rigidbody->getMass());
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
			Collider* collider = new Collider(new Hull(m));
			object->addComponent(collider);
			object->recomputeBoundingBox();
		}
		else if (ResourceManager::getInstance()->loadableResource<Mesh>(hullname))
		{
			m = ResourceManager::getInstance()->getResource<Mesh>(hullname);
			ToolBox::optimizeHullMesh(m);

			Collider* collider = new Collider(new Hull(m));
			object->addComponent(collider);
			object->recomputeBoundingBox();
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

			Collider* collider = new Collider(new Hull(m));
			object->addComponent(collider);
			object->recomputeBoundingBox();
		}
		ResourceManager::getInstance()->release(m);
	}
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
	
	Collider* collider = new Collider(new Sphere(vec4f(0.f), 0.5f * (drawable->getMeshBBMax() - drawable->getMeshBBMin()).getNorm()));
	object->addComponent(collider);
	object->recomputeBoundingBox();
}

void EntityFactory::addComponents(Entity* object, const std::vector<Component*>& components)
{
	for(Component* comp : components)
	{
		object->addComponent(comp, comp->getClassID());
	}
}


bool EntityFactory::addPrefab(std::string prefabName, Entity* prefabObject)
{
	return prefabs.try_emplace(prefabName, prefabObject).second;
}

bool EntityFactory::removePrefab(std::string prefabName)
{
	auto it = prefabs.find(prefabName);
	if (it != prefabs.end())
	{
		world->releaseOwnership(it->second);
		prefabs.erase(it);
		return true;
	}
	return false;
}

bool EntityFactory::containPrefab(std::string prefabName)
{
	return prefabs.find(prefabName) != prefabs.end();
}

bool EntityFactory::loadPrefab(const std::string& resourceDirectory, const std::string& assetPackName, const std::string& fileName)
{
	if (containPrefab(fileName))
		return true;

	// load file and parse JSON
	std::string fullFileName = resourceDirectory + "/Prefabs/"+ assetPackName + "/" + fileName + ".json";
	Variant v; Variant* tmp = nullptr;
	try
	{
		std::ifstream strm(fullFileName.c_str());
		if (!strm.is_open())
			throw std::invalid_argument("Reader::parseFile : Cannot opening file");

		Reader reader(&strm);
		reader.parse(v);
		tmp = &(v.getMap().begin()->second);
	}
	catch (std::exception&)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			std::cerr << "ERROR : loading prefab : " << fileName << " : fail to open or parse file" << std::endl;
		return false;
	}
	Variant& prefabMap = *tmp;
	if (prefabMap.getType() != Variant::MAP)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			std::cerr << "ERROR : loading prefab : " << fileName << " : wrong file formating" << std::endl;
		return false;
	}

	// create and set transform
	Entity* prefab = createEntity();
	prefab->setName(fileName);

	float scale = 1.f;
	if (prefabMap.getMap().find("scale") != prefabMap.getMap().end())
		scale = prefabMap["scale"].toDouble();

	prefab->setWorldTransformation(vec4f(0, 0, 0, 1), scale, quatf(1, 0, 0, 0));
	prefabs.emplace(fileName, prefab);

	// component of prefab
	tryLoadComponents(prefab, &prefabMap, assetPackName);
	prefab->recomputeBoundingBox();

	// end
	return true;
}

Entity* EntityFactory::instantiatePrefab(std::string prefabName, bool _addToScene)
{
	auto it = prefabs.find(prefabName);
	if (it != prefabs.end())
	{
		Entity* copy = createEntity();
		copy->setName(prefabName + " (copy)");
		copy->setWorldTransformation(vec4f(0, 0, 0, 1), it->second->getWorldScale(), quatf(1, 0, 0, 0));

		auto ComponentVisitor = [&](const EntityBase::Element& element)
		{
			if (element.type == DrawableComponent::getStaticClassID())
			{
				const DrawableComponent* original = static_cast<const DrawableComponent*>(element.comp);
				DrawableComponent* drawable = new DrawableComponent(original->getMesh()->name, original->getShader()->name);
				copy->addComponent(drawable);
			}
			return false;
		};

		it->second->allComponentsVisitor(ComponentVisitor);
		copy->recomputeBoundingBox();

		if (_addToScene)
			addToScene(copy);
		return copy;
	}
	return nullptr;
}

void EntityFactory::tryLoadComponents(Entity* object, Variant* variant, const std::string& assetPackName)
{
	if (variant->getType() == Variant::MAP)
	{
		// drawableComponent
		try
		{
			std::string meshName = assetPackName + "/" + (*variant)["drawableComponent"]["meshName"].toString();
			std::string shaderName = (*variant)["drawableComponent"]["shaderName"].toString();

			if (!meshName.empty() && !shaderName.empty())
			{
				if (meshName.find('.') == std::string::npos)
					meshName += ".fbx";

				DrawableComponent* drawable = new DrawableComponent(meshName, shaderName);
				object->addComponent(drawable);
			}
		}
		catch (std::exception&) {}
	}
}