#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "NodeVirtual.h"
#include "VirtualSceneQuerry.h"

class SceneManager
{
	public:
		//	Miscellaneous
		enum CollisionType
		{
			NONE = 0, //!< No collision
			INSIDE,   //!< Object fully inside
			OVERLAP   //!< Shapes are overlapping
		};
		//

		//	Default
		SceneManager();
		SceneManager(const SceneManager& other) = delete;
		SceneManager(SceneManager&& other);
		~SceneManager();

		SceneManager& operator=(const SceneManager& other) = delete;
		SceneManager& operator=(SceneManager&& other);
		//

		//	Public functions
		void init(const glm::vec3& bbMin, const glm::vec3& bbMax, const glm::ivec3& nodeDivision, unsigned int depth);
		void clear();
		void reserveInstanceTrack(const unsigned int& count);
		unsigned int getObjectCount() const;
		//

		//	Object / Entity related
		bool addObject(Entity* object);
		bool removeObject(Entity* object);
		bool updateObject(Entity* object);

		std::vector<Entity*> getAllObjects();
		std::vector<Entity*> getObjectsOnRay(const glm::vec3& position, const glm::vec3& direction, float maxDistance);
		std::vector<Entity*> getObjectsInBox(const glm::vec3& bbMin, const glm::vec3& bbMax);

		void getSceneNodes(VirtualSceneQuerry* collisionTest);
		void getEntities(VirtualSceneQuerry* collisionTest, VirtualEntityCollector* entityCollector);
		//

	private:
		//	Miscellaneous
		struct InstanceTrack
		{
			glm::vec3 position;
			NodeVirtual* owner;
		};

		//	Protected functions
		glm::vec3 getObjectSize(const Entity* entity) const;
		//

		//  Attributes
		std::vector<NodeVirtual*> world;
		std::unordered_map<Entity*, InstanceTrack> instanceTracking;
		//
};
