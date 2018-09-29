#include "World.h"



World::World()
	: entityFactory(this)
{}

World::~World()
{
	// clean sceneManager before entityManager deletion
	sceneManager.clear();
}

void World::setMaxObjectCount(unsigned int count)
{
	sceneManager.reserveInstanceTrack(count);
}

unsigned int World::getObjectCount() const
{
	return entityManager.getObjectCount();
}

bool World::addToScene(Entity* object)
{
	return sceneManager.addObject(object);
}

bool World::updateObject(Entity* object)
{
	return sceneManager.updateObject(object);
}

Entity* World::getNewEntity()
{
	Entity* object = entityManager.getNewEntity();
	object->setParentWorld(this);
	return object;
}

void World::getOwnership(Entity* object)
{
	entityManager.getOwnership(object);
}

void World::releaseOwnership(Entity* object)
{
	entityManager.releaseOwnership(object);
}

void World::clearGarbage()
{
	entityManager.clearGarbage();
}

