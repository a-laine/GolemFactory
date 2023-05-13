#pragma once

#include <EntityComponent/Entity.hpp>

class VirtualEntityCollector
{
	public:
		virtual bool operator() (Entity* entity);
		virtual std::vector<Entity*>& getResult();

		uint64_t m_flags = ~0;
		uint64_t m_exclusionFlags = 0;
		std::vector<Entity*> result;
};