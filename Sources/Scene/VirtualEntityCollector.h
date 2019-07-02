#pragma once

#include "EntityComponent/Entity.hpp"

class VirtualEntityCollector
{
	public:
		virtual bool operator() (Entity* entity);
		std::vector<Entity*>& getResult();

	private:
		std::vector<Entity*> result;
};