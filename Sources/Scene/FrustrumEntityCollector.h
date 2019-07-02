#pragma once

#include "VirtualEntityCollector.h"

class FrustrumEntityCollector : VirtualEntityCollector
{
	public:
		FrustrumEntityCollector();
		bool operator() (Entity* entity);
};