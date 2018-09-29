#pragma once

#include <Scene/SceneManager.h>
#include <World/WorldComponents/EntityFactory.h>
#include <World/WorldComponents/EntityManager.h>



class World
{
	public:
		World();
		World(const World& other) = delete;
		World(World&& other) = delete;
		~World();

		World& operator=(const World& other) = delete;
		World& operator=(World&& other) = delete;

		void setMaxObjectCount(unsigned int count);
		unsigned int getObjectCount() const;

		bool addToScene(Entity* object);
		bool updateObject(Entity* object);
		Entity* getNewEntity();
		void getOwnership(Entity* object);
		void releaseOwnership(Entity* object);

		void clearGarbage();

		SceneManager& getSceneManager() { return sceneManager; }
		const SceneManager& getSceneManager() const { return sceneManager; }
		EntityFactory& getEntityFactory() { return entityFactory; }
		const EntityFactory& getEntityFactory() const { return entityFactory; }

	private:
		SceneManager sceneManager;
		EntityFactory entityFactory;
		EntityManager entityManager;
};

