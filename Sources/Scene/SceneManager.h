#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Utiles/Singleton.h"
#include "NodeVirtual.h"


class SceneManager : public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;

	public:
		enum CollisionType
		{
			NONE = 0, //!< No collision
			INSIDE,   //!< Object fully inside
			OVERLAP   //!< Shapes are overlapping
		};


		void init(const glm::vec3& bbMin, const glm::vec3& bbMax, const glm::ivec3& nodeDivision, unsigned int depth);

		void reserveInstanceTrack(const unsigned int& count);
		unsigned int getObjectCount() const;

		bool addObject(InstanceVirtual* object);
		bool removeObject(InstanceVirtual* object);
		bool updateObject(InstanceVirtual* object);

		void getAllObjects(std::vector<InstanceVirtual*>& result);
		void getObjectsOnRay(std::vector<InstanceVirtual*>& result, const glm::vec3& position, const glm::vec3& direction, float maxDistance);
		void getObjectsInBox(std::vector<InstanceVirtual*>& result, const glm::vec3& bbMin, const glm::vec3& bbMax);

		template<typename CollisionTest, typename EntityCollector>
		void getObjects(EntityCollector& result, CollisionTest collisionTest) {
			if(!world.empty())  getObjects(world[0], result, collisionTest);
		}
		template<typename CollisionTest, typename EntityCollector>
		void getObjectsInBox(EntityCollector& result, CollisionTest collisionTest) {
			if(!world.empty())  getObjectsInBox(world[0], result, collisionTest, testOnlyNodes);
		}

	private:
		struct InstanceTrack
		{
			glm::vec3 position;
			NodeVirtual* owner;
		};


		SceneManager();
		SceneManager(const SceneManager& other) = delete;
		SceneManager(SceneManager&& other) = default;
		~SceneManager();

		SceneManager& operator=(const SceneManager& other) = delete;
		SceneManager& operator=(SceneManager&& other) = default;


		template<typename CollisionTest, typename EntityCollector = EntityList>
		void getObjects(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest);
		template<typename CollisionTest, typename EntityCollector = EntityList>
		void getObjectsInBox(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest);


		//  Attributes
		std::vector<NodeVirtual*> world;
		std::unordered_map<InstanceVirtual*, InstanceTrack> instanceTracking;
};







template<typename CollisionTest, typename EntityCollector>
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


template<typename CollisionTest, typename EntityCollector>
void SceneManager::getObjectsInBox(NodeVirtual* node, EntityCollector& result, CollisionTest collisionTest)
{
	//	initialize and test root in box
	std::vector<NodeVirtual*> fullyInsideNodes;
	std::vector<NodeVirtual*> overlappingNodes;
	CollisionType collision = collisionTest(node);
	switch(collision)
	{
		case INSIDE:  fullyInsideNodes.push_back(node); break;
		case OVERLAP: overlappingNodes.push_back(node); break;
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
			case OVERLAP: overlappingNodes.push_back(node); break;
			default:      break;
		}

		// iterate
		path.back().next(); // node processed
		if(!node->isLeaf() && collision == OVERLAP)
			collisionTest.getChildren(node, path);
	}

	for(NodeVirtual* node : fullyInsideNodes)
		getObjects(node, result, [](NodeVirtual* node) -> CollisionType { return OVERLAP; });

	for(NodeVirtual* node : overlappingNodes)
		node->getObjectList(result);
}


