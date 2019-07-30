#pragma once

#include <vector>

#include <EntityComponent/Entity.hpp>


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
		std::vector<Entity*> garbage;
		int nbObjects;
};

