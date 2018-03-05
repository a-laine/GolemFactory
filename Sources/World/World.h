#pragma once

#include <Scene/SceneManager.h>
#include <Instances/InstanceManager.h>
#include <World/WorldComponents/EntityFactory.h>

#include "Utiles/Singleton.h"



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

		bool updateObject(InstanceVirtual* object);
		bool manageObject(InstanceVirtual* object);
		InstanceVirtual* getObject(InstanceVirtual* object);
		InstanceVirtual* getObject(uint32_t objectId);
		void releaseObject(InstanceVirtual* object);
		void clearGarbage();

		SceneManager& getSceneManager() { return sceneManager; }
		const SceneManager& getSceneManager() const { return sceneManager; }
		EntityFactory& getEntityFactory() { return entityFactory; }
		const EntityFactory& getEntityFactory() const { return entityFactory; }

	private:
		SceneManager sceneManager;
		InstanceManager instanceManager;
		EntityFactory entityFactory;
};

