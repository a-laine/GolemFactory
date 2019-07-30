#pragma once

#include "EntityComponent/Entity.hpp"

class VirtualEntityCollector
{
	friend class Physics;

	public:
		virtual bool operator() (Entity* entity);
		virtual std::vector<Entity*>& getResult();

	protected:
		std::vector<Entity*> result;
};