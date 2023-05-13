#pragma once

#include <vector>

#include "VirtualSceneQuerry.h"

class BoxSceneQuerry : public VirtualSceneQuerry
{
	friend class Physics;

	public:
		BoxSceneQuerry(const vec4f& cornerMin, const vec4f& cornerMax);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		vec4f bbMin;
		vec4f bbMax;
};
