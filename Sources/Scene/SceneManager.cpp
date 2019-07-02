#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_access.hpp>

#include "SceneManager.h"
#include "Utiles/Assert.hpp"



SceneManager::SceneManager(){}
SceneManager::SceneManager(SceneManager&& other)
{
	world = std::move(other.world);
	instanceTracking = std::move(other.instanceTracking);
	other.world.clear();
	other.instanceTracking.clear();
}
SceneManager::~SceneManager()
{
	for (unsigned int i = 0; i < world.size(); i++)
		delete world[i];
}


SceneManager& SceneManager::operator=(SceneManager&& other)
{
	for(unsigned int i = 0; i < world.size(); i++)
		delete world[i];
	world = std::move(other.world);
	instanceTracking = std::move(other.instanceTracking);
	other.world.clear();
	other.instanceTracking.clear();
	return *this;
}


void SceneManager::init(const glm::vec3& bbMin, const glm::vec3& bbMax, const glm::ivec3& nodeDivision, unsigned int depth)
{
	GF_ASSERT(world.empty());
	NodeVirtual* n = new NodeVirtual();
	n->init(bbMin, bbMax, nodeDivision, depth);
	world.push_back(n);
}
void SceneManager::clear()
{
	for(unsigned int i = 0; i < world.size(); i++)
		delete world[i];
	world.clear();
	instanceTracking.clear();
}
void SceneManager::reserveInstanceTrack(const unsigned int& count) { instanceTracking.reserve(count); }
unsigned int SceneManager::getObjectCount() const { return (unsigned int)instanceTracking.size(); }


bool SceneManager::addObject(Entity* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) != 0)
		return false;

	NodeVirtual* node = world[0];
	if(!node->isInside(object->getPosition()))
		return false;

	const glm::vec3 s = getObjectSize(object);
	while (!node->isLeaf() && node->isTooSmall(s))
		node = node->getChildAt(object->getPosition());

	node->addObject(object);
	instanceTracking[object] = {object->getPosition(), node};

	return true;
}

bool SceneManager::removeObject(Entity* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;
	instanceTracking.erase(track);
	
	return node->removeObject(object);
}

bool SceneManager::updateObject(Entity* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;

	const glm::vec3 objectSize = getObjectSize(object);
	if (!node->isInside(object->getPosition()) || node->isTooSmall(objectSize) || node->isTooBig(objectSize))
	{
		node->removeObject(object);

		node = world[0];
		if (!node->isInside(object->getPosition()))
		{
			instanceTracking.erase(track);
			return false;
		}
		else
		{
			while(!node->isLeaf() && node->isTooSmall(objectSize))
				node = node->getChildAt(object->getPosition());
			node->addObject(object);
			track->second.owner = node;
		}
	}
	track->second.position = object->getPosition();
	return true;
}


std::vector<Entity*> SceneManager::getAllObjects()
{
	VirtualSceneQuerry getAllTest;
	VirtualEntityCollector result;

	return std::vector<Entity*>(result.getResult());

	/*if(!world.empty())
	{
		getObjects(world[0], result, [](NodeVirtual*) -> CollisionType { return OVERLAP; });
	}*/
}

std::vector<Entity*> SceneManager::getObjectsOnRay(const glm::vec3& position, const glm::vec3& direction, float maxDistance)
{
	/*if(world.empty())
		return;

	RaySceneQuerry test(position, direction, maxDistance);
	getObjects(world[0], result, test);*/
}

std::vector<Entity*> SceneManager::getObjectsInBox(const glm::vec3& bbMin, const glm::vec3& bbMax)
{
	/*if(world.empty())
		return;

	BoxSceneQuerry test(bbMin, bbMax);
	getObjectsInBox(world[0], result, test);*/
}


void SceneManager::getSceneNodes(VirtualSceneQuerry* collisionTest)
{
	//	initialize and test root
	NodeVirtual* node = world[0];
	CollisionType collision = (CollisionType)(*collisionTest)(node);
	if (collision == NONE)
		return;
	//node->getObjectList(result);

	//	init path and iterate on tree
	std::vector<NodeVirtual::NodeRange> path;
	if (!node->isLeaf())
		node->getChildren(path);
	while (!path.empty())
	{
		if (path.back().empty())
		{
			path.pop_back();
			continue;
		}

		// process node
		node = path.back().get();
		collision = (CollisionType)(*collisionTest)(node);

		//	iterate
		path.back().next(); // node processed
		if (!node->isLeaf() && collision != NONE)
			node->getChildren(path);
	}
}
void SceneManager::getEntities(VirtualSceneQuerry* collisionTest, VirtualEntityCollector* entityCollector)
{
	getSceneNodes(collisionTest);
	std::vector<const NodeVirtual*>& nodeList = collisionTest->getResult();

	for (unsigned int i = 0; i < nodeList.size(); i++)
	{
		const std::vector<Entity*>& entities = nodeList[i]->getEntitiesList();
		for (unsigned int j = 0; j < entities.size(); j++)
		{
			(*entityCollector)(entities[j]);
		}
	}
}


glm::vec3 SceneManager::getObjectSize(const Entity* entity) const
{
	const Shape* Shape = entity->getGlobalBoundingShape();
	if (Shape->type == Shape::ORIENTED_BOX)
	{
		const OrientedBox& box = *static_cast<const OrientedBox*>(Shape);
		return box.max - box.min;
	}
	else if (Shape->type == Shape::AXIS_ALIGNED_BOX)
	{
		const AxisAlignedBox& box = *static_cast<const AxisAlignedBox*>(Shape);
		return box.max - box.min;
	}
	else
	{
		const AxisAlignedBox box = Shape->toAxisAlignedBox();
		return box.max - box.min;
	}
}



//	Physics engine related
NodeVirtual* SceneManager::addSwept(Swept* object)
{
	if (world.empty())
		return nullptr;
	NodeVirtual* node = world[0];
	if (!node->isInside(object->getPosition()))
		return false;
	const glm::vec3 s = object->getSize();
	while (!node->isLeaf() && node->isTooSmall(s))
		node = node->getChildAt(object->getPosition());

	node->addSwept(object);
	return node;
}
//