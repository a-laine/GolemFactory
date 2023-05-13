#pragma once

//#include <algorithm>

#include "Physics/BoundingVolume.h"

namespace Collision2
{
	//	Specialized functions : Axis Aligned Box vs all other (except previous Shape)
	bool collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max);
	bool collide_AxisAlignedBoxvsSphere(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius);
	bool collide_AxisAlignedBoxvsCapsule(const vec4f& boxMin, const vec4f& boxMax, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius);
};
