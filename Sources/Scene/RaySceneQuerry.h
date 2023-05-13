#pragma once

#include "VirtualSceneQuerry.h"

class RaySceneQuerry : public VirtualSceneQuerry
{
	public:
		RaySceneQuerry(const vec4f& pos, const vec4f& dir, float maxDist);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		vec4f position;
		vec4f direction;
		float distance;
};
