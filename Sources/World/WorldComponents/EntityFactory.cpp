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
#include <Renderer/Lighting/LightComponent.h>
#include <Renderer/OccluderComponent.h>
#include <Utiles/ConsoleColor.h>
#include <Animation/Animator.h>
#include <Utiles/ProfilerConfig.h>



EntityFactory::EntityFactory(World* parentWorld)
	: world(parentWorld)
{}

Entity* EntityFactory::createObject(const std::string& type, const vec4f& position, const vec4f& scale, const quatf& orientation, const std::string& name)
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

Entity* EntityFactory::createObject(const std::vector<Component*>& components, const vec4f& position, const vec4f& scale, const quatf& orientation, const std::string& name)
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
	//AnimationComponent* animation = new AnimationComponent(animationName);
	//skeleton->computeCapsules(drawable->getMesh());
	//skeleton->initializeVBOVAO();
	object->addComponent(drawable);
	object->addComponent(skeleton);
	//object->addComponent(animation);
	
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

	SCOPED_CPU_MARKER("EntityFactory::loadPrefab");

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

	vec4f scale = vec4f::one;
	if (prefabMap.getMap().find("scale") != prefabMap.getMap().end())
		scale = vec4f((float)prefabMap["scale"].toDouble());

	prefab->setWorldTransformation(vec4f(0, 0, 0, 1), scale, quatf(1, 0, 0, 0));
	prefabs.emplace(fileName, prefab);

	// component of prefab
	tryLoadComponents(prefab, &prefabMap);
	tryLoadHierarchy(prefab, &prefabMap, assetPackName);
	prefab->recomputeBoundingBox();

	// end
	return true;
}

Entity* EntityFactory::instantiatePrefab(std::string prefabName, bool _addToScene)
{
	auto it = prefabs.find(prefabName);
	if (it != prefabs.end())
	{
		Entity* copy = deepCopy(it->second);
		copy->setWorldTransformation(vec4f(0, 0, 0, 1), it->second->getWorldScale(), quatf(1, 0, 0, 0));
		if (_addToScene)
			addToScene(copy);
		return copy;
	}
	return nullptr;
}

Entity* EntityFactory::deepCopy(Entity* original)
{
	Entity* copy = createEntity();
	copy->setName(original->getName());
	copy->setLocalTransformation(original->getLocalPosition(), original->getLocalScale(), original->getLocalOrientation());

	auto ComponentVisitor = [&](const EntityBase::Element& element)
	{
		if (element.type == DrawableComponent::getStaticClassID())
		{
			const DrawableComponent* original = static_cast<const DrawableComponent*>(element.comp);
			DrawableComponent* copyComp = new DrawableComponent(original);
			copy->addComponent(copyComp);
		}
		else if (element.type == OccluderComponent::getStaticClassID())
		{
			const OccluderComponent* original = static_cast<const OccluderComponent*>(element.comp);
			OccluderComponent* copyComp = new OccluderComponent(original);
			copy->addComponent(copyComp);
		}
		else if (element.type == Collider::getStaticClassID())
		{
			const Collider* original = static_cast<const Collider*>(element.comp);
			Collider* copyComp = new Collider(original);
			copy->addComponent(copyComp);
		}
		return false;
	};

	original->allComponentsVisitor(ComponentVisitor);
	copy->recomputeBoundingBox();

	const auto& children = original->getChilds();
	for (int i = 0; i < children.size(); i++)
	{
		Entity* child = deepCopy(children[i]);
		copy->addChild(child);
		child->setLocalTransformation(child->getLocalPosition(), child->getLocalScale(), child->getLocalOrientation());
	}

	return copy;
}

void EntityFactory::tryLoadTransform(Entity* object, Variant* variant)
{
	// transform load
	vec4f position = vec4f(
		(float)(*variant)["position"].getArray()[0].toDouble(),
		(float)(*variant)["position"].getArray()[1].toDouble(),
		(float)(*variant)["position"].getArray()[2].toDouble(),
		1.f);
	quatf rotation = quatf(
		(float)(*variant)["rotation"].getArray()[3].toDouble(),
		(float)(*variant)["rotation"].getArray()[0].toDouble(),
		(float)(*variant)["rotation"].getArray()[1].toDouble(),
		(float)(*variant)["rotation"].getArray()[2].toDouble());
	rotation.normalize();

	vec4f scale = vec4f::one;
	auto& scaleVariant = (*variant)["scale"];
	if (scaleVariant.getType() == Variant::INT)
		scale = vec4f((float)scaleVariant.toInt());
	else if (scaleVariant.getType() == Variant::FLOAT)
		scale = vec4f(scaleVariant.toFloat());
	else if (scaleVariant.getType() == Variant::DOUBLE)
		scale = vec4f((float)scaleVariant.toDouble());

	object->setLocalTransformation(position, scale, rotation);
}

void EntityFactory::tryLoadComponents(Entity* object, Variant* variant)
{
	// helpers
	std::string messageHeader = "Loading components of " + object->getName();
	if (variant->getType() == Variant::MAP)
	{
		// skeletonComponent always before because :
		//     - drawable component in order to retarget the mesh,
		//     - animation component will do stuff if target entity has skeleton component,
		//     - animator do the same,
		SkeletonComponent* skeleton = nullptr;
		auto it0 = variant->getMap().find("skeletonComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			skeleton = new SkeletonComponent();
			if (skeleton->load(it0->second, object->getName()))
				object->addComponent(skeleton);
			else 
			{
				delete skeleton;
				skeleton = nullptr;
			}
		}

		// drawableComponent
		it0 = variant->getMap().find("drawableComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			DrawableComponent* drawable = new DrawableComponent();
			if (drawable->load(it0->second, object->getName(), skeleton ? skeleton->getSkeleton() : nullptr))
				object->addComponent(drawable);
			else delete drawable;
		}

		// lightComponent
		it0 = variant->getMap().find("lightComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			LightComponent* light = new LightComponent();
			if (light->load(it0->second, object->getName()))
				object->addComponent(light);
			else delete light;
		}

		// occluderComponent
		it0 = variant->getMap().find("occluderComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			OccluderComponent* occluder = new OccluderComponent();
			if (occluder->load(it0->second, object->getName()))
				object->addComponent(occluder);
			else delete occluder;
		}

		// colliderComponent
		it0 = variant->getMap().find("colliderComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			Collider* collider = new Collider();
			if (collider->load(it0->second, object->getName()))
				object->addComponent(collider);
			else delete collider;
		}

		// animationComponent
		it0 = variant->getMap().find("animationComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			AnimationComponent* animation = new AnimationComponent();
			if (animation->load(it0->second, object->getName()))
				object->addComponent(animation);
			else delete animation;
		}

		// animator
		it0 = variant->getMap().find("animator");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			Animator* animator = new Animator();
			if (animator->load(it0->second, object->getName()))
				object->addComponent(animator);
			else delete animator;
		}
	}
}


void EntityFactory::tryLoadHierarchy(Entity* object, Variant* variant, const std::string& assetPackName)
{
	std::string messageHeader = "Loading hierarchy of " + object->getName();
	if (variant->getType() == Variant::MAP)
	{
		auto it = variant->getMap().find("prefabEntities");
		if (it != variant->getMap().end() && it->second.getType() == Variant::ARRAY)
		{
			auto& childrenArray = it->second.getArray();
			for (auto it2 = childrenArray.begin(); it2 != childrenArray.end(); it2++)
			{
				if (it2->getType() == Variant::MAP)
				{
					// load name
					std::string prefabName = "";
					auto prefabNameVariant = it2->getMap().find("prefabName");
					if (prefabNameVariant != it2->getMap().end() && prefabNameVariant->second.getType() == Variant::STRING)
						prefabName = prefabNameVariant->second.toString();

					Entity* child = nullptr;
					if (prefabName.empty())
					{
						child = createEntity();

						// load components and childs
						tryLoadComponents(child, &(*it2));
						tryLoadHierarchy(child, &(*it2), assetPackName);
					}
					else
					{
						loadPrefab(ResourceManager::getInstance()->getRepository(), assetPackName, prefabName);
						child = instantiatePrefab(prefabName);
					}

					auto objectNameVariant = it2->getMap().find("name");
					if (objectNameVariant != it2->getMap().end() && objectNameVariant->second.getType() == Variant::STRING)
						child->setName(objectNameVariant->second.toString());

					// set parent and local transform
					object->addChild(child);
					tryLoadTransform(child, &(*it2));
					child->recomputeBoundingBox();
				}
			}
		}
	}
}

void EntityFactory::printError(std::string header, const char* msg)
{
	if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
	{
		std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : EntityFactory : " << header << " : " << msg << std::flush;
		std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
	}
}

void EntityFactory::printWarning(std::string header, const char* msg)
{
	if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
	{
		std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : EntityFactory : " << header << " : " << msg << std::flush;
		std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
	}
}