#pragma once

#include "Shape.h"

#include "SpecificIntersection/IntersectionPoint.h"
#include "SpecificIntersection/IntersectionSegment.h"
#include "SpecificIntersection/IntersectionTriangle.h"
#include "SpecificIntersection/IntersectionOrientedBox.h"
#include "SpecificIntersection/IntersectionAxisAlignedBox.h"
#include "SpecificIntersection/IntersectionSphere.h"
#include "SpecificIntersection/IntersectionCapsule.h"

namespace Intersection
{
	Result intersect(const Shape& a, const Shape& b);
}