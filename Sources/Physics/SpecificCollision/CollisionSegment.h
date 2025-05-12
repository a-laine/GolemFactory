#pragma once

#include <algorithm>

#include "Physics/BoundingVolume.h"

namespace Collision2
{
	//	Specialized functions : Segment vs all other (except previous Shape)
	bool collide_SegmentvsSegment(const vec3f& segment1a, const vec3f& segment1b, const vec3f& segment2a, const vec3f& segment2b);
	bool collide_SegmentvsTriangle(const vec3f& segment1, const vec3f& segment2, const vec3f& triangle1, const vec3f& triangle2, const vec3f& triangle3, vec3f* intersection = nullptr);
	bool collide_SegmentvsOrientedBox(const vec3f& segment1, const vec3f& segment2, const mat4f& boxTranform, const vec3f& boxMin, const vec3f& boxMax);
	bool collide_SegmentvsAxisAlignedBox(const vec3f& segment1, const vec3f& segment2, const vec3f& boxMin, const vec3f& boxMax);
	bool collide_SegmentvsSphere(const vec3f& segment1, const vec3f& segment2, const vec3f& sphereCenter, const float& sphereRadius);
	bool collide_SegmentvsCapsule(const vec3f& segment1, const vec3f& segment2, const vec3f& capsule1, const vec3f& capsule2, const float& capsuleRadius);
};
