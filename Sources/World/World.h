#pragma once

#include "World/WorldComponents/EntityFactory.h"
#include "World/WorldComponents/EntityManager.h"
#include <Scene/SceneManager.h>
#include <Physics/Physics.h>
#include <Utiles/Singleton.h>
#include <Terrain/TerrainVirtualTexture.h>

class CameraComponent;
class World
{
	public:
		//	Default
		World();
		World(const World& other) = delete;
		World(World&& other) = delete;
		~World();
		//

		//	Operators
		World& operator=(const World& other) = delete;
		World& operator=(World&& other) = delete;
		//

		//	Set / get / add
		void setMaxObjectCount(unsigned int count);
		
		unsigned int getObjectCount() const;
		Entity* getNewEntity();
		void getOwnership(Entity* object);

		SceneManager& getSceneManager();
		const SceneManager& getSceneManager() const;
		EntityFactory& getEntityFactory();
		const EntityFactory& getEntityFactory() const;
		Physics& getPhysics();
		const Physics& getPhysics() const;

		TerrainVirtualTexture& getTerrainVirtualTexture() { return m_terrainVirtualTexture; };

		bool addToScene(Entity* object, int maxDepth = 1000);
		bool removeFromScene(Entity* object);

		void setMainCamera(CameraComponent* _camera);
		CameraComponent* getMainCamera() const;
		//

		//	Public functions
		bool updateObject(Entity* object);
		void releaseOwnership(Entity* object);
		void clearGarbage();
		//


	private:
		Physics physics;
		SceneManager sceneManager;
		EntityFactory entityFactory;
		EntityManager entityManager;
		CameraComponent* m_mainCamera;



		TerrainVirtualTexture m_terrainVirtualTexture;
};

