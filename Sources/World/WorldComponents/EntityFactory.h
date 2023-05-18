#pragma once

#include <string>
#include <vector>
#include <map>
//#include <glm/glm.hpp>
//#include <glm/gtc/quaternion.hpp>
#include "Math/TMath.h"
#include <Utiles/Parser/Reader.h>


class World;
class Entity;
class Component;

class EntityFactory
{
	public:
		explicit EntityFactory(World* parentWorld);

		Entity* createEntity();

		template<typename Callback>
		Entity* createObject(const std::string& type, Callback cb);
		Entity* createObject(const std::string& type, const vec4f& position, const float& scale = 1.f, const quatf& orientation = quatf::identity, const std::string& name = "unknown");

		template<typename Callback>
		Entity* createObject(Callback cb);
		template<typename Callback>
		Entity* createObject(const std::vector<Component*>& components, Callback cb);
		Entity* createObject(const std::vector<Component*>& components, const vec4f& position, const float& scale = 1.f, const quatf& orientation = quatf::identity, const std::string& name = "unknown");

		bool addPrefab(std::string prefabName, Entity* prefabObject);
		bool removePrefab(std::string prefabName);
		bool containPrefab(std::string prefabName);
		bool loadPrefab(const std::string& resourceDirectory, const std::string& assetPackName, const std::string& fileName);
		Entity* instantiatePrefab(std::string prefabName, bool _addToScene = false);

		void tryLoadComponents(Entity* object, Variant* variant, const std::string& assetPackName);

	private:
		Entity* createByType(const std::string& type);

		void addToScene(Entity* object);

		void createDrawable(Entity* object, const std::string& meshName, const std::string& shaderName, const bool& hullGeneration = true);
		void createAnimatable(Entity* object, const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName);
		
		void addComponents(Entity* object, const std::vector<Component*>& components);

		void printError(std::string header, const char* msg);
		void printWarning(std::string header, const char* msg);

		World* world;
		std::map<std::string, Entity*> prefabs;
};



template<typename Callback>
Entity* EntityFactory::createObject(const std::string& type, Callback cb)
{
	Entity* object = createByType(type);
	if(object)
	{
		cb(object);
		addToScene(object);
	}
	return object;
}

template<typename Callback>
Entity* EntityFactory::createObject(Callback cb)
{
	Entity* object = createEntity();
	cb(object);
	addToScene(object);
	return object;
}

template<typename Callback>
Entity* EntityFactory::createObject(const std::vector<Component*>& components, Callback cb)
{
	Entity* object = createEntity();
	addComponents(object, components);
	cb(object);
	addToScene(object);
	return object;
}

