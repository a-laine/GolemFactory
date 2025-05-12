#include "EntityManager.h"

#include <Utiles\Assert.hpp>



EntityManager::EntityManager()
	: nbObjects(0)
{}

EntityManager::~EntityManager()
{
	clearGarbage();
	GF_ASSERT(nbObjects == 0, "Memory leak detected");
}

unsigned int EntityManager::getObjectCount() const
{
	return nbObjects;
}

Entity* EntityManager::getNewEntity()
{
	Entity* object = new Entity();
	object->m_refCount++;
	nbObjects++;
	return object;
}

void EntityManager::getOwnership(Entity* object)
{
    GF_ASSERT(object);
	object->m_refCount++;
}

void EntityManager::releaseOwnership(Entity* object)
{
    GF_ASSERT(object);
	unsigned int previous = object->m_refCount--;
	if(previous == 1)
	{
		//delete object;
		mutexGarbage.lock();
		nbObjects--;
		garbage.push_back(object);
		mutexGarbage.unlock();
	}
}

void EntityManager::clearGarbage()
{
	mutexGarbage.lock();
	for(Entity* object : garbage)
		delete object;
	garbage.clear();
	mutexGarbage.unlock();
}

