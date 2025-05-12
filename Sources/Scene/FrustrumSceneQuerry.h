#pragma once

#include "VirtualSceneQuerry.h"
#include <Physics/Shapes/Hull.h>

class FrustrumSceneQuerry : public VirtualSceneQuerry
{
	public:
		FrustrumSceneQuerry() {};
		FrustrumSceneQuerry(const vec4f& position, const vec4f& direction, const vec4f& verticalDir, const vec4f& leftDir, float verticalAngle, float contextRatio);

		void Set(const vec4f& position, const vec4f& direction, const vec4f& verticalDir, const vec4f& leftDir, float verticalAngle, float contextRatio, float farDistance = 1500.f);
		
		VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override;
		std::vector<const NodeVirtual*>& getResult();

		bool TestSphere(vec4f center, float radius);
		bool TestAABB(vec4f min, vec4f max);


		int maxDepth = 1000;

	private:
		vec4f frustrumPlaneNormals[6];
		vec4f frustrumCorners[5];
};
