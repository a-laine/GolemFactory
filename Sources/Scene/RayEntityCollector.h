#pragma once

#include "VirtualEntityCollector.h"

class RayEntityCollector : public VirtualEntityCollector
{
	public:
		RayEntityCollector(const glm::vec4& pos, const glm::vec4& dir, float maxDist);
		bool operator() (Entity* entity) override;
		std::vector<std::pair<float, unsigned int> >& getSortedResult();

	private:
		glm::vec4 position;
		glm::vec4 direction;
		float distance;
		std::vector<std::pair<float, unsigned int> > sortedIndicies;
};