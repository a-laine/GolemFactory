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
	object->m_refCount--;
	if(object->m_refCount == 0)
	{
		//delete object;
		garbage.push_back(object);
		nbObjects--;
	}
}

void EntityManager::clearGarbage()
{
	for(Entity* object : garbage)
		delete object;
	garbage.clear();
}

