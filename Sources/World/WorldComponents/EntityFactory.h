#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


class World;
class Entity;
class Component;

class EntityFactory
{
	public:
		EntityFactory(World* parentWorld);

		template<typename Callback>
		Entity* createObject(const std::string& type, Callback cb);
		Entity* createObject(const std::string& type, const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.f), const glm::quat& orientation = glm::quat());

		template<typename Callback>
		Entity* createObject(Callback cb);
		template<typename Callback>
		Entity* createObject(const std::vector<Component*>& components, Callback cb);
		Entity* createObject(const std::vector<Component*>& components, const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.f), const glm::quat& orientation = glm::quat());

	private:
		Entity* createEntity();
		Entity* createByType(const std::string& type);

		void addToScene(Entity* object);

		void createDrawable(Entity* object, const std::string& meshName, const std::string& shaderName);
		void createAnimatable(Entity* object, const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName);
		
		void addComponents(Entity* object, const std::vector<Component*>& components);

		World* world;
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

