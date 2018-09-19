#include <algorithm>
#include <glm/gtc/matrix_access.hpp>

#include "SceneManager.h"
#include "Utiles/Assert.hpp"
#include "SceneQueryTests.h"


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


bool SceneManager::addObject(InstanceVirtual* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) != 0)
		return false;

	NodeVirtual* node = world[0];
	if(!node->isInside(object->getPosition()))
		return false;

	const OrientedBox box = object->getBoundingVolume();
	while(!node->isLeaf() && node->isTooSmall(box.max - box.min))
		node = node->getChildAt(object->getPosition());

	node->addObject(object);
	instanceTracking[object] = {object->getPosition(), node};
	return true;
}

bool SceneManager::removeObject(InstanceVirtual* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;
	instanceTracking.erase(track);
	
	return node->removeObject(object);
}

bool SceneManager::updateObject(InstanceVirtual* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;

	const glm::vec3 objectSize = object->getBoundingVolume().max - object->getBoundingVolume().min;
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


void SceneManager::getAllObjects(std::vector<InstanceVirtual*>& result)
{
	if(!world.empty())
	{
		getObjects(world[0], result, [](NodeVirtual*) -> CollisionType { return OVERLAP; });
	}
}

void SceneManager::getObjectsOnRay(std::vector<InstanceVirtual*>& result, const glm::vec3& position, const glm::vec3& direction, float maxDistance)
{
	if(world.empty())
		return;

	DefaultSceneManagerRayTest test(position, direction, maxDistance);
	getObjects(world[0], result, test);
}

void SceneManager::getObjectsInBox(std::vector<InstanceVirtual*>& result, const glm::vec3& bbMin, const glm::vec3& bbMax)
{
	if(world.empty())
		return;

	DefaultSceneManagerBoxTest test(bbMin, bbMax);
	getObjectsInBox(world[0], result, test);
}

