#pragma once

#include <vector>

#include "VirtualSceneQuerry.h"

class BoxSceneQuerry : VirtualSceneQuerry
{
	public:
		BoxSceneQuerry(const glm::vec3& cornerMin, const glm::vec3& cornerMax);

		SceneManager::CollisionType operator() (const NodeVirtual* node) override;
		std::vector<const NodeVirtual*>& getResult();		

	private:
		glm::vec3 bbMin;
		glm::vec3 bbMax;
		std::vector<const NodeVirtual*> result;
};
