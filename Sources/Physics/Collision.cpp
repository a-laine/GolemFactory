#include "Collision.h"

#include <iostream>
#include <string>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>


// https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
// http://www.realtimerendering.com/Collisions.html 


/**	TODO
	- le blindage ne doit etre fais qu'une fois : ex : collide_SegmentvsXXXXXXXX
		le test if(segment1 == segment2) est fais bien en amont pour prevenir la duplication de code

	- l'optimisation se fera en derniers !!!
		- simplifier les appels a projectHalfBox et projectHalfCapsule pour les cas particuliers d'axes
		- try expand function of special case for optimisation (ex: collide_OrientedBoxvsAxisAlignedBox, collide_AxisAlignedBoxvsCapsule, ...)
		- for collision (boolean) SAT : no need to normalize axis (maybe)
**/




//	Private field
namespace
{
	inline bool collide_PointvsShape(const Shape& point, const Shape& b)
	{
		const Point* a = static_cast<const Point*>(&point);
		switch (b.type)
		{
			case Shape::POINT: {
				const Point* c = static_cast<const Point*>(&b);
				return Collision::collide_PointvsPoint(a->p, c->p);
			}
			case Shape::SEGMENT: {
				const Segment* c = static_cast<const Segment*>(&b);
				return Collision::collide_PointvsSegment(a->p, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = static_cast<const Triangle*>(&b);
				return Collision::collide_PointvsTriangle(a->p, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Collision::collide_PointvsOrientedBox(a->p, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_PointvsAxisAlignedBox(a->p, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Collision::collide_PointvsSphere(a->p, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Collision::collide_PointvsCapsule(a->p, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_SegmentvsShape(const Shape& segment, const Shape& b)
	{
		const Segment* a = static_cast<const Segment*>(&segment);
		switch (b.type)
		{
			case Shape::SEGMENT: {
				const Segment* c = static_cast<const Segment*>(&b);
				return Collision::collide_SegmentvsSegment(a->p1, a->p2, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = static_cast<const Triangle*>(&b);
				return Collision::collide_SegmentvsTriangle(a->p1, a->p2, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Collision::collide_SegmentvsOrientedBox(a->p1, a->p2, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_SegmentvsAxisAlignedBox(a->p1, a->p2, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Collision::collide_SegmentvsSphere(a->p1, a->p2, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Collision::collide_SegmentvsCapsule(a->p1, a->p2, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_TrianglevsShape(const Shape& triangle, const Shape& b)
	{
		const Triangle* a = static_cast<const Triangle*>(&triangle);
		switch (b.type)
		{
			case Shape::TRIANGLE: {
				const Triangle* c = static_cast<const Triangle*>(&b);
				return Collision::collide_TrianglevsTriangle(a->p1, a->p2, a->p3, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Collision::collide_TrianglevsOrientedBox(a->p1, a->p2, a->p3, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_TrianglevsAxisAlignedBox(a->p1, a->p2, a->p3, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Collision::collide_TrianglevsSphere(a->p1, a->p2, a->p3, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Collision::collide_TrianglevsCapsule(a->p1, a->p2, a->p3, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_OrientedBoxvsShape(const Shape& obox, const Shape& b)
	{
		const OrientedBox* a = static_cast<const OrientedBox*>(&obox);
		switch (b.type)
		{
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = static_cast<const OrientedBox*>(&b);
				return Collision::collide_OrientedBoxvsOrientedBox(a->base, a->min, a->max, c->base, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_OrientedBoxvsAxisAlignedBox(a->base, a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Collision::collide_OrientedBoxvsSphere(a->base, a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Collision::collide_OrientedBoxvsCapsule(a->base, a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_AxisAlignedBoxvsShape(const Shape& aabox, const Shape& b)
	{
		const AxisAlignedBox* a = static_cast<const AxisAlignedBox*>(&aabox);
		switch (b.type)
		{
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = static_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_AxisAlignedBoxvsAxisAlignedBox(a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Collision::collide_AxisAlignedBoxvsSphere(a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Collision::collide_AxisAlignedBoxvsCapsule(a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_SpherevsShape(const Shape& sphere, const Shape& b)
	{
		const Sphere* a = static_cast<const Sphere*>(&sphere);
		switch (b.type)
		{
			case Shape::SPHERE: {
				const Sphere* c = static_cast<const Sphere*>(&b);
				return Collision::collide_SpherevsSphere(a->center, a->radius, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = static_cast<const Capsule*>(&b);
				return Collision::collide_SpherevsCapsule(a->center, a->radius, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_CapsulevsShape(const Shape& capsule, const Shape& b)
	{
		const Capsule* a = static_cast<const Capsule*>(&capsule);
		if(b.type == Shape::CAPSULE)
		{
			const Capsule* c = static_cast<const Capsule*>(&b);
			return Collision::collide_CapsulevsCapsule(a->p1, a->p2, a->radius, c->p1, c->p2, c->radius);
		}
		else return false;
	};

	//	Debug
	std::string printShapeName(const Shape& shape)
	{
		switch (shape.type)
		{
			case Shape::POINT:				return "point";
			case Shape::SEGMENT:			return "segment";
			case Shape::TRIANGLE:			return "triangle";
			case Shape::ORIENTED_BOX:		return "oriented box";
			case Shape::AXIS_ALIGNED_BOX:	return "axis aligned box";
			case Shape::SPHERE:				return "sphere";
			case Shape::CAPSULE:			return "capsule";
			default:						return "unknown";
		}
	}
	void printError(const std::string& shape1, const std::string& shape2, const int& testNumber)
	{
		std::cout << "Error collision test line " << testNumber << " : (" << shape1 << " vs " << shape2 << ") : return unexpected result." << std::endl;
	}
}
//

//	Public field
bool Collision::collide(const Shape& a, const Shape& b)
{
	//	order objects
	Shape& shape1 = (Shape&) a;
	Shape& shape2 = (Shape&) b;
	if (a.type > b.type) std::swap(shape1, shape2);

	switch (shape1.type)
	{
		case Shape::POINT:				return collide_PointvsShape(shape1, shape2);
		case Shape::SEGMENT:			return collide_SegmentvsShape(shape1, shape2);
		case Shape::TRIANGLE:			return collide_TrianglevsShape(shape1, shape2);
		case Shape::ORIENTED_BOX:		return collide_OrientedBoxvsShape(shape1, shape2);
		case Shape::AXIS_ALIGNED_BOX:	return collide_AxisAlignedBoxvsShape(shape1, shape2);
		case Shape::SPHERE:				return collide_SpherevsShape(shape1, shape2);
		case Shape::CAPSULE:			return collide_CapsulevsShape(shape1, shape2);
		default:						return false;
	}
}
void Collision::debugUnitaryTest(const int& verboseLevel)
{
	// point vs ...
	{
		Point testPoint = Point(glm::vec3(0, 0, 0));
		glm::mat4 dummyTranslate = glm::translate(glm::mat4(1.0), glm::vec3(1, 1, 1));
		glm::mat4 dummyRotate = glm::rotate(0.1f, glm::normalize(glm::vec3(0.5, 1, 3)));

		// ... vs point
		if (Collision::collide(testPoint, Point(glm::vec3(0, 0, 0))) == false && verboseLevel)
			printError("point", "point", __LINE__ - 1);
		if (Collision::collide(testPoint, Point(glm::vec3(0, 0, 1))) == true && verboseLevel)
			printError("point", "point", __LINE__ - 1);

		// ... vs segment
		if (Collision::collide(testPoint, Segment(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1))) == false && verboseLevel)
			printError("point", "segment", __LINE__ - 1);
		if (Collision::collide(testPoint, Segment(glm::vec3(1, 0, -1), glm::vec3(1, 0, 1))) == true && verboseLevel)
			printError("point", "segment", __LINE__ - 1);
		if (Collision::collide(testPoint, Segment(glm::vec3(0, 0, 1), glm::vec3(0, 0, 1))) == true && verboseLevel)
			printError("point", "segment", __LINE__ - 1);
		if (Collision::collide(testPoint, Segment(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0))) == false && verboseLevel)
			printError("point", "segment", __LINE__ - 1);

		// ... vs triangle
		if (Collision::collide(testPoint, Triangle(glm::vec3(-1, 0, -1), glm::vec3(1, 0, -1), glm::vec3(0, 0, 1))) == false && verboseLevel)
			printError("point", "triangle", __LINE__ - 1);
		if (Collision::collide(testPoint, Triangle(glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), glm::vec3(0, 1, 1))) == true && verboseLevel)
			printError("point", "triangle", __LINE__ - 1);
		if (Collision::collide(testPoint, Triangle(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(2, 0, 0))) == false && verboseLevel)
			printError("point", "triangle", __LINE__ - 1);
		if (Collision::collide(testPoint, Triangle(glm::vec3(0, 0, 1), glm::vec3(-1, 0, 1), glm::vec3(2, 0, 1))) == true && verboseLevel)
			printError("point", "triangle", __LINE__ - 1);

		// ... vs OB
		if (Collision::collide(testPoint, OrientedBox(glm::mat4(1.f), glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("point", "oriented box", __LINE__ - 1);
		if (Collision::collide(testPoint, OrientedBox(dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("point", "oriented box", __LINE__ - 1);
		if (Collision::collide(testPoint, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true && verboseLevel)
			printError("point", "oriented box", __LINE__ - 1);
		if (Collision::collide(testPoint, OrientedBox(dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == false && verboseLevel)
			printError("point", "oriented box", __LINE__ - 1);
		if (Collision::collide(testPoint, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == true && verboseLevel)
			printError("point", "oriented box", __LINE__ - 1);

		// ... vs AAB
		if (Collision::collide(testPoint, AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("point", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testPoint, AxisAlignedBox(glm::vec3(0.5f), glm::vec3(1.f))) == true && verboseLevel)
			printError("point", "axis aligned box", __LINE__ - 1);

		// ... vs Sphere
		if (Collision::collide(testPoint, Sphere(glm::vec3(0.f), 1.f)) == false && verboseLevel)
			printError("point", "sphere", __LINE__ - 1);
		if (Collision::collide(testPoint, Sphere(glm::vec3(1.f), 1.f)) == true && verboseLevel)
			printError("point", "sphere", __LINE__ - 1);
		if (Collision::collide(testPoint, Sphere(glm::vec3(0.f), 0.f)) == false && verboseLevel)
			printError("point", "sphere", __LINE__ - 1);
		if (Collision::collide(testPoint, Sphere(glm::vec3(1.f), 0.f)) == true && verboseLevel)
			printError("point", "sphere", __LINE__ - 1);

		// ... vs Capsule
		if (Collision::collide(testPoint, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("point", "capsule", __LINE__ - 1);
		if (Collision::collide(testPoint, Capsule(glm::vec3(2, 0, -1), glm::vec3(1, 0, 3), 0.5f)) == true && verboseLevel)
			printError("point", "capsule", __LINE__ - 1);
		if (Collision::collide(testPoint, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("point", "capsule", __LINE__ - 1);
		if (Collision::collide(testPoint, Capsule(glm::vec3(2, 0, -1), glm::vec3(-1, 0, 3), 2.f)) == false && verboseLevel)
			printError("point", "capsule", __LINE__ - 1);
	}

	// segment vs ...
	{
		Segment testSegment = Segment(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0));
		glm::mat4 dummyTranslate = glm::translate(glm::mat4(1.0), glm::vec3(2, 2, 2));
		glm::mat4 dummyRotate = glm::rotate(0.1f, glm::normalize(glm::vec3(0.5, 1, 3)));

		// ... vs Segment
		if (Collision::collide(testSegment, Segment(glm::vec3(0, -3, 0), glm::vec3(0, 1, 0))) == false && verboseLevel)
			printError("segment", "segment", __LINE__ - 1);
		if (Collision::collide(testSegment, Segment(glm::vec3(0, -3, 1), glm::vec3(0, 1, 1))) == true && verboseLevel)
			printError("segment", "segment", __LINE__ - 1);
		if (Collision::collide(testSegment, testSegment) == false && verboseLevel)
			printError("segment", "segment", __LINE__ - 1);
		if (Collision::collide(testSegment, Segment(glm::vec3(-3, 0, 1), glm::vec3(1, 0, 1))) == true && verboseLevel)
			printError("segment", "segment", __LINE__ - 1);
		if (Collision::collide(testSegment, Segment(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0))) == false && verboseLevel)
			printError("segment", "segment", __LINE__ - 1);
		if (Collision::collide(testSegment, Segment(glm::vec3(0, 0, 1), glm::vec3(0, 0, 1))) == true && verboseLevel)
			printError("segment", "segment", __LINE__ - 1);
		
		// ... vs Triangle
		if (Collision::collide(testSegment, Triangle(glm::vec3(0, -1, -1), glm::vec3(0, 1, -1), glm::vec3(0, 0, 1))) == false && verboseLevel)
			printError("segment", "triangle", __LINE__ - 1);
		if (Collision::collide(testSegment, Triangle(glm::vec3(2, -1, -1), glm::vec3(2, 1, -1), glm::vec3(2, 0, 1))) == true && verboseLevel)
			printError("segment", "triangle", __LINE__ - 1);
		if (Collision::collide(testSegment, Triangle(glm::vec3(0,  1, -1), glm::vec3(0, 3, -1), glm::vec3(0, 2, 1))) == true && verboseLevel)
			printError("segment", "triangle", __LINE__ - 1);
		if (Collision::collide(testSegment, Triangle(glm::vec3(-2, -1, -1), glm::vec3(-2, 1, -1), glm::vec3(-2, 0, 1))) == true && verboseLevel)
			printError("segment", "triangle", __LINE__ - 1);

		// ... vs OB
		if (Collision::collide(testSegment, OrientedBox(glm::mat4(1.f), glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("segment", "oriented box", __LINE__ - 1);
		if (Collision::collide(testSegment, OrientedBox(dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("segment", "oriented box", __LINE__ - 1);
		if (Collision::collide(testSegment, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true && verboseLevel)
			printError("segment", "oriented box", __LINE__ - 1);
		if (Collision::collide(testSegment, OrientedBox(dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == false && verboseLevel)
			printError("segment", "oriented box", __LINE__ - 1);
		if (Collision::collide(testSegment, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == true && verboseLevel)
			printError("segment", "oriented box", __LINE__ - 1);

		// ... vs AAB
		if (Collision::collide(testSegment, AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("segment", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testSegment, AxisAlignedBox(glm::vec3(0.5f), glm::vec3(1.f))) == true && verboseLevel)
			printError("segment", "axis aligned box", __LINE__ - 1);

		// ... vs Sphere
		if (Collision::collide(testSegment, Sphere(glm::vec3(0.f), 1.f)) == false && verboseLevel)
			printError("segment", "sphere", __LINE__ - 1);
		if (Collision::collide(testSegment, Sphere(glm::vec3(1.f), 1.f)) == true && verboseLevel)
			printError("segment", "sphere", __LINE__ - 1);
		if (Collision::collide(testSegment, Sphere(glm::vec3(1.f), 1.5f)) == false && verboseLevel)
			printError("segment", "sphere", __LINE__ - 1);
		if (Collision::collide(testSegment, Sphere(glm::vec3(0.f), 0.f)) == false && verboseLevel)
			printError("segment", "sphere", __LINE__ - 1);
		if (Collision::collide(testSegment, Sphere(glm::vec3(1.f), 0.f)) == true && verboseLevel)
			printError("segment", "sphere", __LINE__ - 1);
		if (Collision::collide(testSegment, Sphere(glm::vec3(0.f, 0.f, 1.f), 1.f)) == false && verboseLevel)
			printError("segment", "sphere", __LINE__ - 1);

		// ... vs Capsule
		if (Collision::collide(testSegment, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("segment", "capsule", __LINE__ - 1);
		if (Collision::collide(testSegment, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, 1, 3), 0.5f)) == true && verboseLevel)
			printError("segment", "capsule", __LINE__ - 1);
		if (Collision::collide(testSegment, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("segment", "capsule", __LINE__ - 1);
		if (Collision::collide(testSegment, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, -1, 3), 2.f)) == false && verboseLevel)
			printError("segment", "capsule", __LINE__ - 1);
	}

	// triangle vs ...
	{
		Triangle testTriangle = Triangle(glm::vec3(-1.f, 0, -1.f), glm::vec3(1.f, 0, -1.f), glm::vec3(0, 0, 1.f));
		glm::mat4 dummyTranslate = glm::translate(glm::mat4(1.0), glm::vec3(2, 2, 2));
		glm::mat4 dummyRotate = glm::rotate(0.1f, glm::normalize(glm::vec3(0.5, 1, 3)));

		// ... vs Triangle
		if (Collision::collide(testTriangle, Triangle(glm::vec3(0, -1, -1), glm::vec3(0, 1, -1), glm::vec3(0, 0, 1))) == false && verboseLevel)
			printError("triangle", "triangle", __LINE__ - 1);
		if (Collision::collide(testTriangle, Triangle(glm::vec3(2, -1, -1), glm::vec3(2, 1, -1), glm::vec3(2, 0, 1))) == true && verboseLevel)
			printError("triangle", "triangle", __LINE__ - 1);
		if (Collision::collide(testTriangle, Triangle(glm::vec3(0, 1, -1), glm::vec3(0, 3, -1), glm::vec3(0, 2, 1))) == true && verboseLevel)
			printError("triangle", "triangle", __LINE__ - 1);
		if (Collision::collide(testTriangle, Triangle(glm::vec3(-2, -1, -1), glm::vec3(-2, 1, -1), glm::vec3(-2, 0, 1))) == true && verboseLevel)
			printError("triangle", "triangle", __LINE__ - 1);

		// ... vs OB
		if (Collision::collide(testTriangle, OrientedBox(glm::mat4(1.f), glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("triangle", "oriented box", __LINE__ - 1);
		if (Collision::collide(testTriangle, OrientedBox(dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("triangle", "oriented box", __LINE__ - 1);
		if (Collision::collide(testTriangle, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true && verboseLevel)
			printError("triangle", "oriented box", __LINE__ - 1);
		if (Collision::collide(testTriangle, OrientedBox(dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == false && verboseLevel)
			printError("triangle", "oriented box", __LINE__ - 1);
		if (Collision::collide(testTriangle, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == true && verboseLevel)
			printError("triangle", "oriented box", __LINE__ - 1);

		// ... vs AAB
		if (Collision::collide(testTriangle, AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("triangle", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testTriangle, AxisAlignedBox(glm::vec3(0.5f), glm::vec3(1.f))) == true && verboseLevel)
			printError("triangle", "axis aligned box", __LINE__ - 1);

		// ... vs Sphere
		if (Collision::collide(testTriangle, Sphere(glm::vec3(0.f), 1.f)) == false && verboseLevel)
			printError("triangle", "sphere", __LINE__ - 1);
		if (Collision::collide(testTriangle, Sphere(glm::vec3(1.f), 1.f)) == true && verboseLevel)
			printError("triangle", "sphere", __LINE__ - 1);
		if (Collision::collide(testTriangle, Sphere(glm::vec3(1.f), 2.f)) == false && verboseLevel)
			printError("triangle", "sphere", __LINE__ - 1);
		if (Collision::collide(testTriangle, Sphere(glm::vec3(0.f), 0.f)) == false && verboseLevel)
			printError("triangle", "sphere", __LINE__ - 1);
		if (Collision::collide(testTriangle, Sphere(glm::vec3(1.f), 0.f)) == true && verboseLevel)
			printError("triangle", "sphere", __LINE__ - 1);
		if (Collision::collide(testTriangle, Sphere(glm::vec3(0.f, 0.f, 1.f), 1.f)) == false && verboseLevel)
			printError("triangle", "sphere", __LINE__ - 1);

		// ... vs Capsule
		if (Collision::collide(testTriangle, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
		if (Collision::collide(testTriangle, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, 1, 3), 0.5f)) == true && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
		if (Collision::collide(testTriangle, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
		if (Collision::collide(testTriangle, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, -1, 3), 2.f)) == false && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
		if (Collision::collide(testTriangle, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 1, 3), 0.5f)) == false && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
		if (Collision::collide(testTriangle, Capsule(glm::vec3(0, -1, -1), glm::vec3(0, 1, 1), 0.f)) == false && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
		
		if (Collision::collide(Triangle(glm::vec3(-10.f, 0, -10.f), glm::vec3(10.f, 0, -10.f), glm::vec3(0, 0, 10.f)), Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.1f)) == false && verboseLevel)
			printError("triangle", "capsule", __LINE__ - 1);
	}

	// oriented box vs ...
	{
		OrientedBox testOrientedBox = OrientedBox(glm::mat4(1.f), glm::vec3(-1.f), glm::vec3(1.f));
		glm::mat4 dummyTranslate = glm::translate(glm::mat4(1.0), glm::vec3(2, 2, 2));
		glm::mat4 dummyRotate = glm::rotate(0.1f, glm::normalize(glm::vec3(0.5, 1, 3)));

		// ... vs OrientedBox
		if (Collision::collide(testOrientedBox, OrientedBox(glm::mat4(1.f), glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("oriented box", "oriented box", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, OrientedBox(dummyTranslate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true && verboseLevel)
			printError("oriented box", "oriented box", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, OrientedBox(dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("oriented box", "oriented box", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, OrientedBox(dummyTranslate*dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true && verboseLevel)
			printError("oriented box", "oriented box", __LINE__ - 1);

		// ... vs AAB
		if (Collision::collide(testOrientedBox, AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("oriented box", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, AxisAlignedBox(glm::vec3(2.f), glm::vec3(3.f))) == true && verboseLevel)
			printError("oriented box", "axis aligned box", __LINE__ - 1);

		// ... vs Sphere
		if (Collision::collide(testOrientedBox, Sphere(glm::vec3(0.f), 1.f)) == false && verboseLevel)
			printError("oriented box", "sphere", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Sphere(glm::vec3(3.f), 1.f)) == true && verboseLevel)
			printError("oriented box", "sphere", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Sphere(glm::vec3(1.f), 0.2f)) == false && verboseLevel)
			printError("oriented box", "sphere", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Sphere(glm::vec3(0.f), 0.f)) == false && verboseLevel)
			printError("oriented box", "sphere", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Sphere(glm::vec3(2.f), 0.f)) == true && verboseLevel)
			printError("oriented box", "sphere", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Sphere(glm::vec3(0.f, 0.f, 1.f), 0.1f)) == false && verboseLevel)
			printError("oriented box", "sphere", __LINE__ - 1);

		// ... vs Capsule
		if (Collision::collide(testOrientedBox, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("oriented box", "capsule", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Capsule(glm::vec3(0, 3, -1), glm::vec3(0, 2, 3), 0.5f)) == true && verboseLevel)
			printError("oriented box", "capsule", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("oriented box", "capsule", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, -1, 3), 2.f)) == false && verboseLevel)
			printError("oriented box", "capsule", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 1, 3), 0.5f)) == false && verboseLevel)
			printError("oriented box", "capsule", __LINE__ - 1);
		if (Collision::collide(testOrientedBox, Capsule(glm::vec3(0, -1, -1), glm::vec3(0, 1, 1), 0.f)) == false && verboseLevel)
			printError("oriented box", "capsule", __LINE__ - 1);
	}

	// axis aligned box vs ...
	{
		AxisAlignedBox testAAB = AxisAlignedBox(glm::vec3(-1.f), glm::vec3(1.f));

		// ... vs AAB
		if (Collision::collide(testAAB, AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f))) == false && verboseLevel)
			printError("axis aligned box", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testAAB, AxisAlignedBox(glm::vec3(2.f), glm::vec3(3.f))) == true && verboseLevel)
			printError("axis aligned box", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testAAB, AxisAlignedBox(glm::vec3(-1.f, -1.f, 1.f), glm::vec3(1.f, 1.f, 2.f))) == false && verboseLevel)
			printError("axis aligned box", "axis aligned box", __LINE__ - 1);
		if (Collision::collide(testAAB, AxisAlignedBox(glm::vec3(1.f), glm::vec3(3.f))) == false && verboseLevel)
			printError("axis aligned box", "axis aligned box", __LINE__ - 1);

		// ... vs Sphere
		if (Collision::collide(testAAB, Sphere(glm::vec3(0.f), 1.f)) == false && verboseLevel)
			printError("axis aligned box", "sphere", __LINE__ - 1);
		if (Collision::collide(testAAB, Sphere(glm::vec3(3.f), 1.f)) == true && verboseLevel)
			printError("axis aligned box", "sphere", __LINE__ - 1);
		if (Collision::collide(testAAB, Sphere(glm::vec3(1.f), 0.2f)) == false && verboseLevel)
			printError("axis aligned box", "sphere", __LINE__ - 1);
		if (Collision::collide(testAAB, Sphere(glm::vec3(0.f), 0.f)) == false && verboseLevel)
			printError("axis aligned box", "sphere", __LINE__ - 1);
		if (Collision::collide(testAAB, Sphere(glm::vec3(2.f), 0.f)) == true && verboseLevel)
			printError("axis aligned box", "sphere", __LINE__ - 1);
		if (Collision::collide(testAAB, Sphere(glm::vec3(0.f, 0.f, 1.f), 0.1f)) == false && verboseLevel)
			printError("axis aligned box", "sphere", __LINE__ - 1);

		// ... vs Capsule
		if (Collision::collide(testAAB, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("axis aligned box", "capsule", __LINE__ - 1);
		if (Collision::collide(testAAB, Capsule(glm::vec3(0, 3, -1), glm::vec3(0, 2, 3), 0.5f)) == true && verboseLevel)
			printError("axis aligned box", "capsule", __LINE__ - 1);
		if (Collision::collide(testAAB, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("axis aligned box", "capsule", __LINE__ - 1);
		if (Collision::collide(testAAB, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, -1, 3), 2.f)) == false && verboseLevel)
			printError("axis aligned box", "capsule", __LINE__ - 1);
		if (Collision::collide(testAAB, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 1, 3), 0.5f)) == false && verboseLevel)
			printError("axis aligned box", "capsule", __LINE__ - 1);
		if (Collision::collide(testAAB, Capsule(glm::vec3(0, -1, -1), glm::vec3(0, 1, 1), 0.f)) == false && verboseLevel)
			printError("axis aligned box", "capsule", __LINE__ - 1);
	}

	// shere vs ...
	{
		Sphere testSphere = Sphere(glm::vec3(0.f), 1.f);

		// ... vs Sphere
		if (Collision::collide(testSphere, Sphere(glm::vec3(0.f), 1.f)) == false && verboseLevel)
			printError("sphere", "sphere", __LINE__ - 1);
		if (Collision::collide(testSphere, Sphere(glm::vec3(3.f), 1.f)) == true && verboseLevel)
			printError("sphere", "sphere", __LINE__ - 1);
		if (Collision::collide(testSphere, Sphere(glm::vec3(0.5f), 0.5f)) == false && verboseLevel)
			printError("sphere", "sphere", __LINE__ - 1);

		// ... vs Capsule
		if (Collision::collide(testSphere, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("sphere", "capsule", __LINE__ - 1);
		if (Collision::collide(testSphere, Capsule(glm::vec3(0, 3, -1), glm::vec3(0, 2, 3), 0.5f)) == true && verboseLevel)
			printError("sphere", "capsule", __LINE__ - 1);
		if (Collision::collide(testSphere, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("sphere", "capsule", __LINE__ - 1);
		if (Collision::collide(testSphere, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, -1, 3), 2.f)) == false && verboseLevel)
			printError("sphere", "capsule", __LINE__ - 1);
		if (Collision::collide(testSphere, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 1, 3), 0.5f)) == false && verboseLevel)
			printError("sphere", "capsule", __LINE__ - 1);
		if (Collision::collide(testSphere, Capsule(glm::vec3(0, -1, -1), glm::vec3(0, 1, 1), 0.f)) == false && verboseLevel)
			printError("sphere", "capsule", __LINE__ - 1);
	}

	// capsule vs ...
	{
		Capsule testCapsule = Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), 1.f);

		// ... vs Capsule
		if (Collision::collide(testCapsule, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false && verboseLevel)
			printError("capsule", "capsule", __LINE__ - 1);
		if (Collision::collide(testCapsule, Capsule(glm::vec3(0, 3, -1), glm::vec3(0, 2, 3), 0.5f)) == true && verboseLevel)
			printError("capsule", "capsule", __LINE__ - 1);
		if (Collision::collide(testCapsule, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false && verboseLevel)
			printError("capsule", "capsule", __LINE__ - 1);
		if (Collision::collide(testCapsule, Capsule(glm::vec3(0, 2, -1), glm::vec3(0, -1, 3), 2.f)) == false && verboseLevel)
			printError("capsule", "capsule", __LINE__ - 1);
		if (Collision::collide(testCapsule, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 1, 3), 0.5f)) == false && verboseLevel)
			printError("capsule", "capsule", __LINE__ - 1);
		if (Collision::collide(testCapsule, Capsule(glm::vec3(0, -1, -1), glm::vec3(0, 1, 1), 0.f)) == false && verboseLevel)
			printError("capsule", "capsule", __LINE__ - 1);
	}
}
//
