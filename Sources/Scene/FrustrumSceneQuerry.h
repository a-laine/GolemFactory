#pragma once

#include "VirtualSceneQuerry.h"
#include <Physics/Shapes/Hull.h>

class FrustrumSceneQuerry : public VirtualSceneQuerry
{
	public:
		FrustrumSceneQuerry(const vec4f& position, const vec4f& direction, const vec4f& verticalDir, const vec4f& leftDir, float verticalAngle, float contextRatio);

		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;
		std::vector<const NodeVirtual*>& getResult();

	private:
		vec4f frustrumPlaneNormals[6];
		vec4f frustrumCorners[5];
};
