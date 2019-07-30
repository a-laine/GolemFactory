#pragma once

#include "VirtualEntityCollector.h"

class FrustrumEntityCollector : public VirtualEntityCollector
{
	public:
		FrustrumEntityCollector();
		bool operator() (Entity* entity) override;
		std::vector<Entity*>& getResult() override;
};