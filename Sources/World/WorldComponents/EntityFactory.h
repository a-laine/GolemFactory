#pragma once

#include <string>
#include <glm/glm.hpp>


class World;
class InstanceVirtual;
class InstanceDrawable;
class InstanceAnimatable;

class EntityFactory
{
	public:
		EntityFactory(World* parentWorld);

		template<typename Callback>
		InstanceVirtual* createObject(const std::string& type, Callback cb);
		InstanceVirtual* createObject(const std::string& type, const glm::vec3& position, const glm::vec3& scale);
		InstanceVirtual* createObject(const std::string& type, const glm::vec3& position, const glm::vec3& scale, const glm::mat4& orientation);

	private:
		InstanceVirtual* createByType(const std::string& type);
		void addToScene(InstanceVirtual* object);

		InstanceDrawable* createDrawable(const std::string& meshName, const std::string& shaderName);
		InstanceAnimatable* createAnimatable(const std::string& meshName, const std::string& shaderName);
		InstanceAnimatable* createAnimatable(const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName);
		//InstanceContainer* createContainer();

		World* world;
};


template<typename Callback>
InstanceVirtual* EntityFactory::createObject(const std::string& type, Callback cb)
{
	InstanceVirtual* object = createByType(type);
	if(object)
	{
		cb(object);
		addToScene(object);
	}
	return object;
}

