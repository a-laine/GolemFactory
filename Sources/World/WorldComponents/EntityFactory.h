#pragma once

#include <string>
#include <glm/glm.hpp>


class World;
class InstanceVirtual;

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
		
		InstanceVirtual* createDrawable(const std::string& meshName, const std::string& shaderName);
		//InstanceVirtual* createAnimatable(const std::string& meshName, const std::string& shaderName);
		InstanceVirtual* createAnimatable(const std::string& meshName, const std::string& skeletonName, const std::string& animationName, const std::string& shaderName);

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

