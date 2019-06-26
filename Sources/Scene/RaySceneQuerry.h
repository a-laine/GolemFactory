#pragma once

#include "VirtualSceneQuerry.h"

class RaySceneQuerry : VirtualSceneQuerry
{
	public:
		RaySceneQuerry(const glm::vec3& pos, const glm::vec3& dir, float maxDist);

		SceneManager::CollisionType operator() (const NodeVirtual* node) override;
		std::vector<const NodeVirtual*>& getResult();

	private:
		glm::vec3 position;
		glm::vec3 direction;
		float distance;
		std::vector<const NodeVirtual*> result;
};
