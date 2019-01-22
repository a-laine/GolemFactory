#pragma once

#include "BoundingVolume.h"

#include "SpecificCollision/CollisionPoint.h"
#include "SpecificCollision/CollisionSegment.h"
#include "SpecificCollision/CollisionTriangle.h"
#include "SpecificCollision/CollisionOrientedBox.h"
#include "SpecificCollision/CollisionAxisAlignedBox.h"
#include "SpecificCollision/CollisionSphere.h"
#include "SpecificCollision/CollisionCapsule.h"


namespace Collision
{
	bool collide(const Shape& a, const Shape& b);
	void debugUnitaryTest(const int& verboseLevel = 0);
};
