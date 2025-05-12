#pragma once

#include <algorithm>

#include "Physics/BoundingVolume.h"
#include "Physics/CollisionReport.h"

namespace Collision
{
	//	Specialized functions : point vs all other
	bool collide_PointvsPoint(const vec3f& point1, const vec3f& point2, CollisionReport* report = nullptr);
	bool collide_PointvsSegment(const vec3f& point, const vec3f& segment1, const vec3f& segment2, CollisionReport* report = nullptr);
	bool collide_SegmentvsPoint(const vec3f& point, const vec3f& segment1, const vec3f& segment2, CollisionReport* report = nullptr);
	bool collide_PointvsTriangle(const vec3f& point, const vec3f& triangle1, const vec3f& triangle2, const vec3f& triangle3, CollisionReport* report = nullptr);
	bool collide_TrianglevsPoint(const vec3f& point, const vec3f& triangle1, const vec3f& triangle2, const vec3f& triangle3, CollisionReport* report = nullptr);

	bool collide_PointvsOrientedBox(const vec3f& point, const mat4f& boxTranform, const vec3f& boxMin, const vec3f& boxMax, CollisionReport* report = nullptr);
	bool collide_PointvsAxisAlignedBox(const vec3f& point, const vec3f& boxMin, const vec3f& boxMax, CollisionReport* report = nullptr);
	bool collide_PointvsSphere(const vec3f& point, const vec3f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
	bool collide_PointvsCapsule(const vec3f& point, const vec3f& capsule1, const vec3f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
	bool collide_PointvsHull(const vec3f& point, const std::vector<vec3f>& hullPoints, const std::vector<vec3f>& hullNormals, const std::vector<unsigned short>& hullFaces, const mat4f& hullBase, CollisionReport* report = nullptr);
};


