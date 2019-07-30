#pragma once

#include "VirtualSceneQuerry.h"

class RaySceneQuerry : public VirtualSceneQuerry
{
	public:
		RaySceneQuerry(const glm::vec3& pos, const glm::vec3& dir, float maxDist);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		glm::vec3 position;
		glm::vec3 direction;
		float distance;
};
