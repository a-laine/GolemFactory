#pragma once

#include <vector>

#include "VirtualSceneQuerry.h"

class BoxSceneQuerry : public VirtualSceneQuerry
{
	friend class Physics;

	public:
		BoxSceneQuerry();
		BoxSceneQuerry(const vec4f& cornerMin, const vec4f& cornerMax);

		void Set(const vec4f& cornerMin, const vec4f& cornerMax);

		bool TestAABB(vec4f min, vec4f max);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;

	private:
		vec4f bbMin;
		vec4f bbMax;
};
