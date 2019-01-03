#include "Intersection.h"

#include <iostream>

//	Private field
namespace
{
	inline Intersection::Result intersect_PointvsShape(const Shape& point, const Shape& b)
	{
		const Point* a = static_cast<const Point*>(&point);
		switch (b.type)
		{
			case Shape::POINT: {
				const Point* c = static_cast<const Point*>(&b);
				return Intersection::intersect_PointvsPoint(a->p, c->p);
			}
			case Shape::SEGMENT: {
				const Segment* c = static_cast<const Segment*>(&b);
				return Intersection::intersect_PointvsSegment(a->p, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = static_cast<const Triangle*>(&b);
				return Intersection::intersect_PointvsTriangle(a->p, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Intersection::intersect_PointvsOrientedBox(a->p, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_PointvsAxisAlignedBox(a->p, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Intersection::intersect_PointvsSphere(a->p, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Intersection::intersect_PointvsCapsule(a->p, c->p1, c->p2, c->radius);
			}
			default: return Intersection::Result();
		}
	};
	inline Intersection::Result intersect_SegmentvsShape(const Shape& segment, const Shape& b)
	{
		const Segment* a = static_cast<const Segment*>(&segment);
		switch (b.type)
		{
			case Shape::SEGMENT: {
				const Segment* c = static_cast<const Segment*>(&b);
				return Intersection::intersect_SegmentvsSegment(a->p1, a->p2, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = static_cast<const Triangle*>(&b);
				return Intersection::intersect_SegmentvsTriangle(a->p1, a->p2, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Intersection::intersect_SegmentvsOrientedBox(a->p1, a->p2, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_SegmentvsAxisAlignedBox(a->p1, a->p2, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Intersection::intersect_SegmentvsSphere(a->p1, a->p2, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Intersection::intersect_SegmentvsCapsule(a->p1, a->p2, c->p1, c->p2, c->radius);
			}
			default: return Intersection::Result();
		}
	};
	inline Intersection::Result intersect_TrianglevsShape(const Shape& triangle, const Shape& b)
	{
		const Triangle* a = static_cast<const Triangle*>(&triangle);
		switch (b.type)
		{
			case Shape::TRIANGLE: {
				const Triangle* c = static_cast<const Triangle*>(&b);
				return Intersection::intersect_TrianglevsTriangle(a->p1, a->p2, a->p3, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Intersection::intersect_TrianglevsOrientedBox(a->p1, a->p2, a->p3, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_TrianglevsAxisAlignedBox(a->p1, a->p2, a->p3, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Intersection::intersect_TrianglevsSphere(a->p1, a->p2, a->p3, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Intersection::intersect_TrianglevsCapsule(a->p1, a->p2, a->p3, c->p1, c->p2, c->radius);
			}
			default: return Intersection::Result();
		}
	};
	inline Intersection::Result intersect_OrientedBoxvsShape(const Shape& box, const Shape& b)
	{
		const OrientedBox* a = static_cast<const OrientedBox*>(&box);
		switch (b.type)
		{
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Intersection::intersect_OrientedBoxvsOrientedBox(a->base, a->min, a->max, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_OrientedBoxvsAxisAlignedBox(a->base, a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Intersection::intersect_OrientedBoxvsSphere(a->base, a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Intersection::intersect_OrientedBoxvsCapsule(a->base, a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return Intersection::Result();
		}
	};
	inline Intersection::Result intersect_AxisAlignedBoxvsShape(const Shape& box, const Shape& b)
	{
		const AxisAlignedBox* a = static_cast<const AxisAlignedBox*>(&box);
		switch (b.type)
		{
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_AxisAlignedBoxvsAxisAlignedBox(a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Intersection::intersect_AxisAlignedBoxvsSphere(a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Intersection::intersect_AxisAlignedBoxvsCapsule(a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return Intersection::Result();
		}
	};
	inline Intersection::Result intersect_SpherevsShape(const Shape& sphere, const Shape& b)
	{
		const Sphere* a = static_cast<const Sphere*>(&sphere);
		switch (b.type)
		{
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Intersection::intersect_SpherevsSphere(a->center, a->radius, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Intersection::intersect_SpherevsCapsule(a->center, a->radius, c->p1, c->p2, c->radius);
			}
			default: return Intersection::Result();
		}
	};
	inline Intersection::Result intersect_CapsulevsShape(const Shape& capsule, const Shape& b)
	{
		const Capsule* a = static_cast<const Capsule*>(&capsule);
		if (b.type == Shape::CAPSULE)
		{
			const Capsule* c = static_cast<const Capsule*>(&b);
			return Intersection::intersect_CapsulevsCapsule(a->p1, a->p2, a->radius, c->p1, c->p2, c->radius);
		}
		else return Intersection::Result();
	};
}
//


//	Public field
Intersection::Result Intersection::intersect(const Shape& a, const Shape& b)
{
	//	order objects
	Shape& shape1 = (Shape&)a;
	Shape& shape2 = (Shape&)b;
	if (a.type > b.type) std::swap(shape1, shape2);

	switch (shape1.type)
	{
		case Shape::POINT:				return intersect_PointvsShape(shape1, shape2);
		case Shape::SEGMENT:			return intersect_SegmentvsShape(shape1, shape2);
		case Shape::TRIANGLE:			return intersect_TrianglevsShape(shape1, shape2);
		case Shape::ORIENTED_BOX:		return intersect_OrientedBoxvsShape(shape1, shape2);
		case Shape::AXIS_ALIGNED_BOX:	return intersect_AxisAlignedBoxvsShape(shape1, shape2);
		case Shape::SPHERE:				return intersect_SpherevsShape(shape1, shape2);
		case Shape::CAPSULE:			return intersect_CapsulevsShape(shape1, shape2);
		default:						return Intersection::Result();
	}
}
//