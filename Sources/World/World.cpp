#include "World.h"



World::World()
	: entityFactory(this)
{}

World::~World()
{
	// clean sceneManager before instanceManager deletion
	sceneManager.clear();
}

void World::setMaxObjectCount(unsigned int count)
{
	instanceManager.setMaxNumberOfInstances(count);
	sceneManager.reserveInstanceTrack(count);
}

unsigned int World::getObjectCount() const
{
	return instanceManager.getNumberOfInstances();
}

bool World::manageObject(InstanceVirtual* object)
{
	bool ok = instanceManager.add(object);
	if(ok) object->setParentWorld(this);
	return ok;
}

InstanceVirtual* World::getObject(InstanceVirtual* object)
{
	return instanceManager.get(object->getId());
}

InstanceVirtual* World::getObject(uint32_t objectId)
{
	return instanceManager.get(objectId);
}

void World::releaseObject(InstanceVirtual* object)
{
	instanceManager.release(object);
}

bool World::updateObject(InstanceVirtual* object)
{
	return sceneManager.updateObject(object);
}

void World::clearGarbage()
{
	instanceManager.clearGarbage();
}

