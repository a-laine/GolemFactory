#pragma once

#include <vector>

#include "VirtualSceneQuerry.h"

class BoxSceneQuerry : public VirtualSceneQuerry
{
	friend class Physics;

	public:
		BoxSceneQuerry(const glm::vec4& cornerMin, const glm::vec4& cornerMax);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		glm::vec4 bbMin;
		glm::vec4 bbMax;
};
