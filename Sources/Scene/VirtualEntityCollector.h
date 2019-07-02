#pragma once

#include "EntityComponent/Entity.hpp"

class VirtualEntityCollector
{
	public:
		virtual bool operator() (Entity* entity);
		virtual std::vector<Entity*>& getResult();

	protected:
		std::vector<Entity*> result;
};