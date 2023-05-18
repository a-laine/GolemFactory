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
#include <Utiles/ConsoleColor.cpp>



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
		/*Entity* copy = createEntity();
		copy->setName(prefabName + " (copy)");

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
		copy->recomputeBoundingBox();*/


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
			DrawableComponent* drawable = new DrawableComponent(original->getMesh()->name, original->getShader()->name);
			copy->addComponent(drawable);
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
		(*variant)["position"].getArray()[0].toDouble(),
		(*variant)["position"].getArray()[1].toDouble(),
		(*variant)["position"].getArray()[2].toDouble(),
		1.f);
	quatf rotation = quatf(
		(*variant)["rotation"].getArray()[3].toDouble(),
		(*variant)["rotation"].getArray()[0].toDouble(),
		(*variant)["rotation"].getArray()[1].toDouble(),
		(*variant)["rotation"].getArray()[2].toDouble());
	rotation.normalize();

	float scale = 1.f;
	auto& scaleVariant = (*variant)["scale"];
	if (scaleVariant.getType() == Variant::INT)
		scale = scaleVariant.toInt();
	else if (scaleVariant.getType() == Variant::FLOAT)
		scale = scaleVariant.toFloat();
	else if (scaleVariant.getType() == Variant::DOUBLE)
		scale = scaleVariant.toDouble();

	object->setLocalTransformation(position, scale, rotation);
}

void EntityFactory::tryLoadComponents(Entity* object, Variant* variant, const std::string& assetPackName)
{
	// helpers
	const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
	{
		if (variant.getMap().find(label) != variant.getMap().end())
		{
			auto& v = variant[label];
			if (v.getType() == Variant::FLOAT)
				destination = v.toFloat();
			else if (v.getType() == Variant::DOUBLE)
				destination = v.toDouble();
			else if (v.getType() == Variant::INT)
				destination = v.toInt();
			else
				return false;
			return true;
		}
		return false;
	};
	const auto TryLoadAsVec4f = [](Variant& variant, const char* label, vec4f& destination)
	{
		int sucessfullyParsed = 0;
		auto it0 = variant.getMap().find(label);
		if (it0 != variant.getMap().end() && it0->second.getType() == Variant::ARRAY)
		{
			auto varray = it0->second.getArray();
			vec4f parsed = destination;
			for (int i = 0; i < 4 && i < varray.size(); i++)
			{
				auto& element = varray[i];
				if (element.getType() == Variant::FLOAT)
				{
					parsed[i] = element.toFloat();
					sucessfullyParsed++;
				}
				else if (element.getType() == Variant::DOUBLE)
				{
					parsed[i] = element.toDouble();
					sucessfullyParsed++;
				}
				else if (element.getType() == Variant::INT)
				{
					parsed[i] = element.toInt();
					sucessfullyParsed++;
				}
			}
			destination = parsed;
		}
		return sucessfullyParsed;
	};
	const auto TryLoadInt = [](Variant& variant, const char* label, int& destination)
	{
		auto it0 = variant.getMap().find(label);
		if (it0 != variant.getMap().end() && it0->second.getType() == Variant::INT)
		{
			destination = it0->second.toInt();
			return true;
		}
		return false;
	};

	std::string messageHeader = "Loading components of " + object->getName();
	if (variant->getType() == Variant::MAP)
	{
		// drawableComponent
		auto it0 = variant->getMap().find("drawableComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			std::string secondHeader = " : DrawableComponent";

			std::string meshName, shaderName, subMeshName;
			auto it1 = it0->second.getMap().find("meshName");
			if (it1 != it0->second.getMap().end() && it1->second.getType() == Variant::STRING)
			{
				meshName = it1->second.toString();
				if (!meshName.empty())
					meshName = assetPackName + "/" + meshName;
				if (meshName.find('.') == std::string::npos)
					meshName += ".fbx";
			}

			it1 = it0->second.getMap().find("subMeshName");
			if (it1 != it0->second.getMap().end() && it1->second.getType() == Variant::STRING)
			{
				meshName = it1->second.toString();
				/*if (ResourceManager::getInstance()->findResource<Mesh>(it1->second.toString()))
				{
					meshName = it1->second.toString();
				}

				meshName = it1->second.toString();
				if (!meshName.empty())
					meshName = assetPackName + "/" + meshName;
				if (meshName.find('.') == std::string::npos)
					meshName += ".fbx";*/
			}

			it1 = it0->second.getMap().find("shaderName");
			if (it1 != it0->second.getMap().end() && it1->second.getType() == Variant::STRING)
				shaderName = it1->second.toString();

			if (!meshName.empty() && !shaderName.empty())
			{
				DrawableComponent* drawable = new DrawableComponent(meshName, shaderName);
				object->addComponent(drawable);
			}
			else
			{
				if (meshName.empty())
					printError(messageHeader + secondHeader, "no mesh name");
				if (shaderName.empty())
					printError(messageHeader + secondHeader, "no shader name");
			}
		}

		// lightComponent
		it0 = variant->getMap().find("lightComponent");
		if (it0 != variant->getMap().end() && it0->second.getType() == Variant::MAP)
		{
			std::string secondHeader = " : LightComponent";
			vec4f color = vec4f(1.f, 1.f, 1.f, 1.f);
			float range = 10;
			float intensity = 1;
			float inCutOff = 28;
			float outCutOff = 32;
			int type = 0;

			bool rangeOk = TryLoadAsFloat(it0->second, "range", range);
			bool intensityOk = TryLoadAsFloat(it0->second, "intensity", intensity);
			int colorOk = TryLoadAsVec4f(it0->second, "color", color);
			bool typeOk = TryLoadInt(it0->second, "type", type);
			bool inCutoffOk = TryLoadAsFloat(it0->second, "inCutOff", inCutOff);
			bool outCutoffOk = TryLoadAsFloat(it0->second, "outCutOff", outCutOff);

			if (!typeOk)
				printError(messageHeader + secondHeader, "fail parsing light type");
			else if ((type != 0) && (type != 1))
			{
				printWarning(messageHeader + secondHeader, "invalid light type, valid are 0=point, 1=spot. Was set to 0");
				type = 0;
			}
			else if (type == 1)
			{
				if (!inCutoffOk)
					printWarning(messageHeader + secondHeader, "fail parsing spot in cutoff angle, Was set to 28");
				if (!outCutoffOk)
					printWarning(messageHeader + secondHeader, "fail parsing spot in cutoff angle, Was set to 32");
			}

			if (!rangeOk)
				printWarning(messageHeader + secondHeader, "fail parsing light range, was set to 10");
			if (!intensityOk)
				printWarning(messageHeader + secondHeader, "fail parsing light intensity, was set to 1");
			if (colorOk < 3)
				printWarning(messageHeader + secondHeader, "fail parsing light color, was set to white");
			else
			{
				float maxValue = std::max(color.x, std::max(color.y, color.z));
				if (maxValue > 1.f)
					color *= 1.f / 255.f;
				color.w = 1.f;
			}
			
			if (typeOk)
			{
				LightComponent* light = new LightComponent();
				light->setColor(color);
				light->setRange(range);
				light->setIntensity(intensity);
				light->setPointLight(type == 0);

				if (type == 1)
				{
					light->setInnerCutOffAngle(std::min(inCutOff, outCutOff));
					light->setOuterCutOffAngle(std::max(inCutOff, outCutOff));
				}

				object->addComponent(light);
			}
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
						tryLoadComponents(child, &(*it2), assetPackName);
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
					//child->setLocalTransformation(child->getLocalPosition(), child->getLocalScale(), child->getLocalOrientation());
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