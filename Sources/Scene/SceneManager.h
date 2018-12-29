#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "NodeVirtual.h"


class SceneManager
{
	public:
		enum CollisionType
		{
			NONE = 0, //!< No collision
			INSIDE,   //!< Object fully inside
			OVERLAP   //!< Shapes are overlapping
		};


		SceneManager();
		SceneManager(const SceneManager& other) = delete;
		SceneManager(SceneManager&& other);
		~SceneManager();

		SceneManager& operator=(const SceneManager& other) = delete;
		SceneManager& operator=(SceneManager&& other);

		void init(const glm::vec3& bbMin, const glm::vec3& bbMax, const glm::ivec3& nodeDivision, unsigned int depth);
		void clear();

		void reserveInstanceTrack(const unsigned int& count);
		unsigned int getObjectCount() const;

		bool addObject(Entity* object);
		bool removeObject(Entity* object);
		bool updateObject(Entity* object);

		void getAllObjects(std::vector<Entity*>& result);
		void getObjectsOnRay(std::vector<Entity*>& result, const glm::vec3& position, const glm::vec3& direction, float maxDistance);
		void getObjectsInBox(std::vector<Entity*>& result, const glm::vec3& bbMin, const glm::vec3& bbMax);

		template<typename EntityCollector, typename CollisionTest>
		void getObjects(EntityCollector& result, CollisionTest collisionTest) {
			if(!world.empty())  getObjects(world[0], result, collisionTest);
		}
		template<typename EntityCollector, typename CollisionTest>
		void getObjectsInBox(EntityCollector& result, CollisionTest collisionTest) {
			if(!world.empty())  getObjectsInBox(world[0], result, collisionTest, testOnlyNodes);
		}

	private:
		struct InstanceTrack
		{
			glm::vec3 position;
			NodeVirtual* owner;
		};

		glm::vec3 getObjectSize(const Entity* entity) const;

		template<typename EntityCollector, typename CollisionTest>
		void getObjects(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest);
		template<typename EntityCollector, typename CollisionTest>
		void getObjectsInBox(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest);


		//  Attributes
		std::vector<NodeVirtual*> world;
		std::unordered_map<Entity*, InstanceTrack> instanceTracking;
};







template<typename EntityCollector, typename CollisionTest>
void SceneManager::getObjects(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest)
{
	//	initialize and test root
	CollisionType collision = (CollisionType) collisionTest(node);
	if(collision == NONE)
		return;
	node->getObjectList(result);

	//	init path and iterate on tree
	std::vector<NodeVirtual::NodeRange> path;
	if(!node->isLeaf())
		node->getChildren(path);
	while(!path.empty())
	{
		if(path.back().empty())
		{
			path.pop_back();
			continue;
		}

		// process node
		node = path.back().get();
		collision = (CollisionType) collisionTest(node);
		if(collision != NONE)
			node->getObjectList(result);

		//	iterate
		path.back().next(); // node processed
		if(!node->isLeaf() && collision != NONE)
			node->getChildren(path);
	}
}


template<typename EntityCollector, typename CollisionTest>
void SceneManager::getObjectsInBox(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest)
{
	//	initialize and test root in box
	std::vector<NodeVirtual*> fullyInsideNodes;
	CollisionType collision = collisionTest(node);
	switch(collision)
	{
		case INSIDE:  fullyInsideNodes.push_back(node); break;
		case OVERLAP: node->getObjectList(result); break;
		default:      return;
	}

	//	init path and iterate on tree
	std::vector<NodeVirtual::NodeRange> path;
	if(!node->isLeaf())
		collisionTest.getChildren(node, path);
	while(!path.empty())
	{
		if(path.back().empty())
		{
			path.pop_back();
			continue;
		}

		// process node
		node = path.back().get();
		collision = collisionTest(node);
		switch(collision)
		{
			case INSIDE:  fullyInsideNodes.push_back(node); break;
			case OVERLAP: node->getObjectList(result); break;
			default:      break;
		}

		// iterate
		path.back().next(); // node processed
		if(!node->isLeaf() && collision == OVERLAP)
			collisionTest.getChildren(node, path);
	}

	for(NodeVirtual* node : fullyInsideNodes)
		getObjects(node, result, [](NodeVirtual*) -> CollisionType { return OVERLAP; });
}


