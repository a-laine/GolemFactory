#include "Intersection.h"
#include "GJK.h"
#include "Resources/Mesh.h"

#include <iostream>


//	Private field
namespace
{
	inline Intersection::Contact intersect_PointvsShape(const Shape& point, const Shape& b)
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
			case Shape::HULL: {
				const Hull* c = static_cast<const Hull*>(&b);
				return Intersection::intersect_PointvsHull(a->p, *c->mesh->getVertices(), *c->mesh->getNormals(), *c->mesh->getFaces(), c->base);
			}
			default: return Intersection::Contact();
		}
	};
	inline Intersection::Contact intersect_SegmentvsShape(const Shape& segment, const Shape& b)
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
			default: return Intersection::Contact();
		}
	};
	inline Intersection::Contact intersect_TrianglevsShape(const Shape& triangle, const Shape& b)
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
			default: return Intersection::Contact();
		}
	};
	inline Intersection::Contact intersect_OrientedBoxvsShape(const Shape& box, const Shape& b)
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
			default: return Intersection::Contact();
		}
	};
	inline Intersection::Contact intersect_AxisAlignedBoxvsShape(const Shape& box, const Shape& b)
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
			default: return Intersection::Contact();
		}
	};
	inline Intersection::Contact intersect_SpherevsShape(const Shape& sphere, const Shape& b)
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
			default: return Intersection::Contact();
		}
	};
	inline Intersection::Contact intersect_CapsulevsShape(const Shape& capsule, const Shape& b)
	{
		const Capsule* a = static_cast<const Capsule*>(&capsule);
		if (b.type == Shape::CAPSULE)
		{
			const Capsule* c = static_cast<const Capsule*>(&b);
			return Intersection::intersect_CapsulevsCapsule(a->p1, a->p2, a->radius, c->p1, c->p2, c->radius);
		}
		else return Intersection::Contact();
	};
}
//


//	Public field
Intersection::Contact Intersection::intersect(const Shape& a, const Shape& b)
{
	//	order objects
	Shape& Shape1 = (Shape&)a;
	Shape& Shape2 = (Shape&)b;
	bool swaped = false;
	if (a.type > b.type) 
	{
		std::swap(Shape1, Shape2);
		swaped = true;
	}

	switch (Shape1.type)
	{
		case Shape::POINT:
			if(swaped) return intersect_PointvsShape(Shape1, Shape2).swap();
			else return intersect_PointvsShape(Shape1, Shape2);
		case Shape::SEGMENT:
			if (swaped) return intersect_SegmentvsShape(Shape1, Shape2).swap();
			else return intersect_SegmentvsShape(Shape1, Shape2);
		case Shape::TRIANGLE:
			if (swaped) return intersect_TrianglevsShape(Shape1, Shape2).swap();
			else return intersect_TrianglevsShape(Shape1, Shape2);
		case Shape::ORIENTED_BOX:		
			if (swaped) return intersect_OrientedBoxvsShape(Shape1, Shape2).swap();
			else return intersect_OrientedBoxvsShape(Shape1, Shape2);
		case Shape::AXIS_ALIGNED_BOX:	
			if (swaped) return intersect_AxisAlignedBoxvsShape(Shape1, Shape2).swap();
			else return intersect_AxisAlignedBoxvsShape(Shape1, Shape2);
		case Shape::SPHERE:	
			if (swaped) return intersect_SpherevsShape(Shape1, Shape2).swap();
			else return intersect_SpherevsShape(Shape1, Shape2);
		case Shape::CAPSULE:			
			if (swaped) return intersect_CapsulevsShape(Shape1, Shape2).swap();
			else return intersect_CapsulevsShape(Shape1, Shape2);
		default:						
			return Intersection::Contact();
	}
}
//