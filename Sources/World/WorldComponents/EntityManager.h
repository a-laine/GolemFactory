#pragma once

#include <vector>

#include <EntityComponent/Entity.hpp>
#include <Utiles/Mutex.h>


class EntityManager
{
	public:
		EntityManager();
		~EntityManager();

		unsigned int getObjectCount() const;

		Entity* getNewEntity();
		void getOwnership(Entity* object);
		void releaseOwnership(Entity* object);

		void clearGarbage();

	private:
		Mutex mutexGarbage;
		std::vector<Entity*> garbage;
		std::atomic_uint nbObjects;
};

