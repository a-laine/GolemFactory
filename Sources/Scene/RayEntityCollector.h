#pragma once

#include "VirtualEntityCollector.h"

class RayEntityCollector : public VirtualEntityCollector
{
	public:
		RayEntityCollector(const glm::vec3& pos, const glm::vec3& dir, float maxDist);
		bool operator() (Entity* entity) override;
		std::vector<std::pair<float, unsigned int> >& getSortedResult();

	private:
		glm::vec3 position;
		glm::vec3 direction;
		float distance;
		std::vector<std::pair<float, unsigned int> > sortedIndicies;
};