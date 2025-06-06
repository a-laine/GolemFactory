#include "World.h"

//	Default
World::World() : entityFactory(this), m_mainCamera(nullptr)
{}

World::~World()
{
	// clean sceneManager before entityManager deletion
	sceneManager.clear();
}
//

//	Public functions
bool World::updateObject(Entity* object)
{
	return sceneManager.updateObject(object);
}
void World::releaseOwnership(Entity* object)
{
	entityManager.releaseOwnership(object);
}
void World::clearGarbage()
{
	entityManager.clearGarbage();
}
//


//	Set / get / add
void World::setMaxObjectCount(unsigned int count)
{
	sceneManager.reserveInstanceTrack(count);
}


unsigned int World::getObjectCount() const
{
	return entityManager.getObjectCount();
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


SceneManager& World::getSceneManager() { return sceneManager; }
const SceneManager& World::getSceneManager() const { return sceneManager; }
EntityFactory& World::getEntityFactory() { return entityFactory; }
const EntityFactory& World::getEntityFactory() const { return entityFactory; }
Physics& World::getPhysics() { return physics; }
const Physics& World::getPhysics() const { return physics; }


bool World::addToScene(Entity* object, int maxDepth)
{
	physics.addMovingEntity(object);
	return sceneManager.addObject(object, maxDepth);
}
bool World::removeFromScene(Entity* object)
{
	physics.removeMovingEntity(object);
	return sceneManager.removeObject(object);
}


void World::setMainCamera(CameraComponent* _camera)
{
	m_mainCamera = _camera;
}
CameraComponent* World::getMainCamera() const
{
	return m_mainCamera;
}
//