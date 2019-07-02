#pragma once

#include <vector>

#include "VirtualSceneQuerry.h"

class BoxSceneQuerry : public VirtualSceneQuerry
{
	friend class Physics;

	public:
		BoxSceneQuerry(const glm::vec3& cornerMin, const glm::vec3& cornerMax);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		glm::vec3 bbMin;
		glm::vec3 bbMax;
};
