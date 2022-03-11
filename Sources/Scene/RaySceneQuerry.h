#pragma once

#include "VirtualSceneQuerry.h"

class RaySceneQuerry : public VirtualSceneQuerry
{
	public:
		RaySceneQuerry(const glm::vec4& pos, const glm::vec4& dir, float maxDist);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		glm::vec4 position;
		glm::vec4 direction;
		float distance;
};
