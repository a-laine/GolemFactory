#pragma once

#include "VirtualEntityCollector.h"

class RayEntityCollector : public VirtualEntityCollector
{
	public:
		RayEntityCollector(const vec4f& pos, const vec4f& dir, float maxDist);
		bool operator() (Entity* entity) override;
		std::vector<std::pair<float, unsigned int> >& getSortedResult();

	private:
		vec4f position;
		vec4f direction;
		float distance;
		std::vector<std::pair<float, unsigned int> > sortedIndicies;
};