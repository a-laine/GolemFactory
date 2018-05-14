#include "Collision.h"

#include <iostream>
#include <string>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

#define EPSILON 0.0001f

// https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
// http://www.realtimerendering.com/Collisions.html 



/**	TODO
	- le blindage ne doit etre fais qu'une fois : ex : collide_SegmentvsXXXXXXXX
		le test if(segment1 == segment2) est fais bien en amont pour prevenir la duplication de code

	- l'optimisation se fera en derniers !!!
**/

//	Private field
namespace
{
	inline bool collide_PointvsShape(const Shape& point, const Shape& b)
	{
		const Point* a = reinterpret_cast<const Point*>(&point);
		switch (b.type)
		{
			case Shape::POINT: {
				const Point* c = reinterpret_cast<const Point*>(&b);
				return Collision::collide_PointvsPoint(a->p, c->p);
			}
			case Shape::SEGMENT: {
				const Segment* c = reinterpret_cast<const Segment*>(&b);
				return Collision::collide_PointvsSegment(a->p, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = reinterpret_cast<const Triangle*>(&b);
				return Collision::collide_PointvsTriangle(a->p, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Collision::collide_PointvsOrientedBox(a->p, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_PointvsAxisAlignedBox(a->p, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Collision::collide_PointvsSphere(a->p, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Collision::collide_PointvsCapsule(a->p, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_SegmentvsShape(const Shape& segment, const Shape& b)
	{
		const Segment* a = reinterpret_cast<const Segment*>(&segment);
		switch (b.type)
		{
			case Shape::SEGMENT: {
				const Segment* c = reinterpret_cast<const Segment*>(&b);
				return Collision::collide_SegmentvsSegment(a->p1, a->p2, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = reinterpret_cast<const Triangle*>(&b);
				return Collision::collide_SegmentvsTriangle(a->p1, a->p2, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Collision::collide_SegmentvsOrientedBox(a->p1, a->p2, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_SegmentvsAxisAlignedBox(a->p1, a->p2, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Collision::collide_SegmentvsSphere(a->p1, a->p2, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Collision::collide_SegmentvsCapsule(a->p1, a->p2, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_TrianglevsShape(const Shape& triangle, const Shape& b)
	{
		const Triangle* a = reinterpret_cast<const Triangle*>(&triangle);
		switch (b.type)
		{
			case Shape::TRIANGLE: {
				const Triangle* c = reinterpret_cast<const Triangle*>(&b);
				return Collision::collide_TrianglevsTriangle(a->p1, a->p2, a->p3, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Collision::collide_TrianglevsOrientedBox(a->p1, a->p2, a->p3, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_TrianglevsAxisAlignedBox(a->p1, a->p2, a->p3, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Collision::collide_TrianglevsSphere(a->p1, a->p2, a->p3, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Collision::collide_TrianglevsCapsule(a->p1, a->p2, a->p3, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_OrientedBoxvsShape(const Shape& obox, const Shape& b)
	{
		const OrientedBox* a = reinterpret_cast<const OrientedBox*>(&obox);
		switch (b.type)
		{
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Collision::collide_OrientedBoxvsOrientedBox(a->transform, a->min, a->max, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_OrientedBoxvsAxisAlignedBox(a->transform, a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Collision::collide_OrientedBoxvsSphere(a->transform, a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Collision::collide_OrientedBoxvsCapsule(a->transform, a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_AxisAlignedBoxvsShape(const Shape& aabox, const Shape& b)
	{
		const AxisAlignedBox* a = reinterpret_cast<const AxisAlignedBox*>(&aabox);
		switch (b.type)
		{
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Collision::collide_AxisAlignedBoxvsAxisAlignedBox(a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Collision::collide_AxisAlignedBoxvsSphere(a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Collision::collide_AxisAlignedBoxvsCapsule(a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_SpherevsShape(const Shape& sphere, const Shape& b)
	{
		const Sphere* a = reinterpret_cast<const Sphere*>(&sphere);
		switch (b.type)
		{
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Collision::collide_SpherevsSphere(a->center, a->radius, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Collision::collide_SpherevsCapsule(a->center, a->radius, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool collide_CapsulevsShape(const Shape& capsule, const Shape& b)
	{
		const Capsule* a = reinterpret_cast<const Capsule*>(&capsule);
		if(b.type == Shape::CAPSULE)
		{
			const Capsule* c = reinterpret_cast<const Capsule*>(&b);
			return Collision::collide_CapsulevsCapsule(a->p1, a->p2, a->radius, c->p1, c->p2, c->radius);
		}
		else return false;
	};

	//	Utils
	inline float projectBox(const glm::vec3& axis, const glm::vec3& boxHalfSize)
	{
		//	axis is in absolute base
		//	boxHalfSize is in box local space (origin is box center)
		return std::abs(boxHalfSize.x*axis.x) + std::abs(boxHalfSize.y*axis.y) + std::abs(boxHalfSize.z*axis.z);
	}
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
	inline void printError(const Shape& shape1, const Shape& shape2, const int& testNumber)
	{
		std::cout << "Error collision test " << testNumber << " : (" << printShapeName(shape1) << " vs " << printShapeName(shape2) << ") : return unexpected result."<< std::endl;
	}
	inline void printError(const std::string& shape1, const std::string& shape2, const int& testNumber)
	{
		std::cout << "Error collision test " << testNumber << " : (" << shape1 << " vs " << shape2 << ") : return unexpected result." << std::endl;
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
		if (Collision::collide(testPoint, Point(glm::vec3(0, 0, 0))) == false)
			printError("point", "point", 1);
		if (Collision::collide(testPoint, Point(glm::vec3(0, 0, 1))) == true) 
			printError("point", "point", 2);

		// ... vs segment
		if (Collision::collide(testPoint, Segment(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1))) == false)
			printError("point", "segment", 3);
		if (Collision::collide(testPoint, Segment(glm::vec3(1, 0, -1), glm::vec3(1, 0, 1))) == true)
			printError("point", "segment", 4);
		if (Collision::collide(testPoint, Segment(glm::vec3(0, 0, 1), glm::vec3(0, 0, 1))) == true)
			printError("point", "segment", 5);
		if (Collision::collide(testPoint, Segment(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0))) == false)
			printError("point", "segment", 6);

		// ... vs triangle
		if (Collision::collide(testPoint, Triangle(glm::vec3(-1, 0, -1), glm::vec3(1, 0, -1), glm::vec3(0, 0, 1))) == false)
			printError("point", "triangle", 7);
		if (Collision::collide(testPoint, Triangle(glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), glm::vec3(0, 1, 1))) == true)
			printError("point", "triangle", 8);
		if (Collision::collide(testPoint, Triangle(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(2, 0, 0))) == false)
			printError("point", "triangle", 9);
		if (Collision::collide(testPoint, Triangle(glm::vec3(0, 0, 1), glm::vec3(-1, 0, 1), glm::vec3(2, 0, 1))) == true)
			printError("point", "triangle", 10);

		// ... vs OB
		if (Collision::collide(testPoint, OrientedBox(glm::mat4(1.f), glm::vec3(-0.5f), glm::vec3(0.5f))) == false)
			printError("point", "oriented box", 11);
		if (Collision::collide(testPoint, OrientedBox(dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == false)
			printError("point", "oriented box", 12);
		if (Collision::collide(testPoint, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true)
			printError("point", "oriented box", 13);
		if (Collision::collide(testPoint, OrientedBox(dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == false)
			printError("point", "oriented box", 14);
		if (Collision::collide(testPoint, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == true)
			printError("point", "oriented box", 15);

		// ... vs AAB
		if (Collision::collide(testPoint, AxisAlignedBox(glm::vec3(-0.5f), glm::vec3(0.5f))) == false)
			printError("point", "axis aligned box", 16);
		if (Collision::collide(testPoint, AxisAlignedBox(glm::vec3(0.5f), glm::vec3(1.f))) == true)
			printError("point", "axis aligned box", 17);

		// ... vs Sphere
		if (Collision::collide(testPoint, Sphere(glm::vec3(0.f), 1.f)) == false)
			printError("point", "sphere", 18);
		if (Collision::collide(testPoint, Sphere(glm::vec3(1.f), 1.f)) == true)
			printError("point", "sphere", 19);
		if (Collision::collide(testPoint, Sphere(glm::vec3(0.f), 0.f)) == false)
			printError("point", "sphere", 20);
		if (Collision::collide(testPoint, Sphere(glm::vec3(1.f), 0.f)) == true)
			printError("point", "sphere", 21);

		// ... vs Capsule
		if (Collision::collide(testPoint, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.5f)) == false)
			printError("point", "capsule", 22);
		if (Collision::collide(testPoint, Capsule(glm::vec3(2, 0, -1), glm::vec3(1, 0, 3), 0.5f)) == true)
			printError("point", "capsule", 23);
		if (Collision::collide(testPoint, Capsule(glm::vec3(0, 0, -1), glm::vec3(0, 0, 3), 0.f)) == false)
			printError("point", "capsule", 24);
		if (Collision::collide(testPoint, Capsule(glm::vec3(2, 0, -1), glm::vec3(-1, 0, 3), 2.f)) == false)
			printError("point", "capsule", 25);
	}

	// segment vs ...
	{
		Segment testSegment = Segment(glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0));
		glm::mat4 dummyTranslate = glm::translate(glm::mat4(1.0), glm::vec3(2, 2, 2));
		glm::mat4 dummyRotate = glm::rotate(0.1f, glm::normalize(glm::vec3(0.5, 1, 3)));

		// ... vs Segment
		if (Collision::collide(testSegment, Segment(glm::vec3(0, -3, 0), glm::vec3(0, 1, 0))) == false)
			printError("segment", "segment", 1);
		if (Collision::collide(testSegment, Segment(glm::vec3(0, -3, 1), glm::vec3(0, 1, 1))) == true)
			printError("segment", "segment", 2);
		if (Collision::collide(testSegment, testSegment) == false)
			printError("segment", "segment", 3);
		if (Collision::collide(testSegment, Segment(glm::vec3(-3, 0, 1), glm::vec3(1, 0, 1))) == true)
			printError("segment", "segment", 4);
		if (Collision::collide(testSegment, Segment(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0))) == false)
			printError("segment", "segment", 5);
		if (Collision::collide(testSegment, Segment(glm::vec3(0, 0, 1), glm::vec3(0, 0, 1))) == true)
			printError("segment", "segment", 6);
		
		// ... vs Triangle
		if (Collision::collide(testSegment, Triangle(glm::vec3(0, -1, -1), glm::vec3(0, 1, -1), glm::vec3(0, 0, 1))) == false)
			printError("segment", "triangle", 7);
		if (Collision::collide(testSegment, Triangle(glm::vec3(2, -1, -1), glm::vec3(2, 1, -1), glm::vec3(2, 0, 1))) == true)
			printError("segment", "triangle", 8);
		if (Collision::collide(testSegment, Triangle(glm::vec3(0,  1, -1), glm::vec3(0, 3, -1), glm::vec3(0, 2, 1))) == true)
			printError("segment", "triangle", 9);
		if (Collision::collide(testSegment, Triangle(glm::vec3(-2, -1, -1), glm::vec3(-2, 1, -1), glm::vec3(-2, 0, 1))) == true)
			printError("segment", "triangle", 10);

		// ... vs OB
		if (Collision::collide(testSegment, OrientedBox(glm::mat4(1.f), glm::vec3(-0.5f), glm::vec3(0.5f))) == false)
			printError("segment", "oriented box", 11);
		if (Collision::collide(testSegment, OrientedBox(dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == false)
			printError("segment", "oriented box", 12);
		if (Collision::collide(testSegment, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(-0.5f), glm::vec3(0.5f))) == true)
			printError("segment", "oriented box", 13);
		if (Collision::collide(testSegment, OrientedBox(dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == false)
			printError("segment", "oriented box", 14);
		if (Collision::collide(testSegment, OrientedBox(dummyTranslate * dummyRotate, glm::vec3(0.f), glm::vec3(0.f))) == true)
			printError("segment", "oriented box", 15);
	}
}
//

//	Specialized functions : point
bool Collision::collide_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2)
{
	return point1 == point2;
}
bool Collision::collide_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2)
{
	glm::vec3 s = segment2 - segment1;
	if (s == glm::vec3(0.f)) return point == segment1;
	else
	{
		glm::vec3 u = glm::normalize(s);
		glm::vec3 u2 = point - segment1;
		glm::vec3 u3 = u2 - glm::dot(u, u2) * u; // distance of point to segment
		return glm::dot(u3, u3) <= EPSILON && glm::dot(u, u2) <= glm::length(s) && glm::dot(u, u2) >= EPSILON;
	}
}
bool Collision::collide_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	//	check if point is coplanar to triangle
	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(u1, u2);
	glm::vec3 p = point - triangle1;

	if(n == glm::vec3(0.f)) // flat triangle
	{
		glm::vec3 u3 = triangle3 - triangle2;
		float d1 = glm::dot(u1, u1);
		float d2 = glm::dot(u2, u2);
		float d3 = glm::dot(u3, u3);
		
		if (d1 >= d2 && d1 >= d3) return collide_PointvsSegment(point, triangle1, triangle2);
		else if (d2 >= d1 && d2 >= d3) return collide_PointvsSegment(point, triangle1, triangle3);
		else return collide_PointvsSegment(point, triangle3, triangle2);
	}
	else if (glm::dot(p, n) <= EPSILON)
	{
		glm::normalize(n);

		//	checking barycentric coordinates
		float crossDot = glm::dot(u1, u2);
		float magnitute = glm::dot(u1, u1)*glm::dot(u2, u2) - crossDot*crossDot;
		glm::vec2 barry;
		barry.x = (glm::dot(u2, u2) * glm::dot(p, u1) - crossDot * glm::dot(p, u2)) / magnitute;
		barry.y = (glm::dot(u1, u1) * glm::dot(p, u2) - crossDot * glm::dot(p, u1)) / magnitute;
		return !(barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f);
	}
	else return false;
}
bool Collision::collide_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	glm::vec3 bmin = glm::vec3(boxTranform*glm::vec4(boxMin, 1.f));
	glm::vec3 bdiag = glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) - bmin;
	glm::vec3 bx = glm::normalize(glm::vec3(boxTranform*glm::vec4(1.f, 0.f, 0.f, 0.f)));
	glm::vec3 by = glm::normalize(glm::vec3(boxTranform*glm::vec4(0.f, 1.f, 0.f, 0.f)));
	glm::vec3 bz = glm::normalize(glm::vec3(boxTranform*glm::vec4(0.f, 0.f, 1.f, 0.f)));

	glm::vec3 p = point - bmin;
	if (glm::dot(p, bx) < -EPSILON || glm::dot(p, by) < -EPSILON || glm::dot(p, bz) < -EPSILON) return false;
	else if (glm::dot(p, bx) > glm::dot(bdiag, bx) + EPSILON || glm::dot(p, by) > glm::dot(bdiag, by) + EPSILON || glm::dot(p, bz) > glm::dot(bdiag, bz) + EPSILON) return false;
	else return true;
}
bool Collision::collide_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	if(point.x < boxMin.x || point.y < boxMin.y || point.z < boxMin.z) return false;
	else if (point.x > boxMax.x || point.y > boxMax.y || point.z > boxMax.z) return false;
	else return true;
}
bool Collision::collide_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	glm::vec3 u = point - sphereCenter;
	return glm::length(u) <= std::max(sphereRadius, EPSILON);
}
bool Collision::collide_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	glm::vec3 s = capsule2 - capsule1;
	if (glm::dot(s, s) == 0.f) return glm::length(point - capsule1) <= std::max(capsuleRadius, EPSILON);
	else
	{
		glm::vec3 u = glm::normalize(s);
		glm::vec3 u2 = point - capsule1;
		glm::vec3 u3 = u2 - glm::dot(u, u2) * u; // distance of point to segment
		return glm::dot(u3, u3) <= std::max(capsuleRadius, EPSILON) && glm::dot(u, u2) <= glm::length(s) + capsuleRadius && glm::dot(u, u2) >= -capsuleRadius;
	}
}
//

//	Specialized functions : segment
bool Collision::collide_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	glm::vec3 s1 = segment1b - segment1a;
	glm::vec3 s2 = segment2b - segment2a;
	glm::vec3 n = glm::cross(s1, s2);

	if (n == glm::vec3(0.f))	// parallel or one segment is a point
	{
		if (s1 == glm::vec3(0.f))
			return collide_PointvsSegment(segment1a, segment2a, segment2b);
		else if (s2 == glm::vec3(0.f))
			return collide_PointvsSegment(segment2a, segment1a, segment1b);
		else // segment are parallel
		{
			glm::vec3 u1 = glm::normalize(s1);
			glm::vec3 u3 = segment1a - segment2a;
			glm::vec3 d = u3 - u1 * std::abs(glm::dot(u3, u1));
			return glm::dot(d, d) <= EPSILON;
		}
	}
	else
	{
		glm::vec3 u1 = glm::normalize(s1);
		glm::vec3 u2 = glm::normalize(s2);
		n = glm::normalize(n);
		float t1 = -glm::determinant(glm::mat3(segment1a - segment2a, u2, n)) / glm::dot(n, n);
		float t2 = -glm::determinant(glm::mat3(segment1a - segment2a, u1, n)) / glm::dot(n, n);
		 
		glm::vec3 d = segment2a + u2*t2 - (segment1a + u1*t1);
		return glm::dot(d, d) <= EPSILON;
	}
}
bool Collision::collide_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	//	begin and eliminate special cases
	glm::vec3 v1 = triangle2 - triangle1;
	glm::vec3 v2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(v1, v2);

	if (n == glm::vec3(0.f)) // flat triangle
	{
		glm::vec3 v3 = triangle3 - triangle2;
		float d1 = glm::dot(v1, v1);
		float d2 = glm::dot(v2, v2);
		float d3 = glm::dot(v3, v3);

		if (d1 >= d2 && d1 >= d3) return collide_SegmentvsSegment(segment1, segment2, triangle1, triangle2);
		else if (d2 >= d1 && d2 >= d3) return collide_SegmentvsSegment(segment1, segment2, triangle1, triangle3);
		else return collide_SegmentvsSegment(segment1, segment2, triangle3, triangle2);
	}

	//	compute intersection point between ray and plane
	glm::vec3 s = segment2 - segment1;
	if (s == glm::vec3(0.f)) return collide_PointvsTriangle(segment1, triangle1, triangle2, triangle3);

	n = glm::normalize(n);
	if (glm::dot(n, s) == 0.f) return false; // segment parallel to triangle plane
	glm::vec3 u = glm::normalize(s);

	float depth = glm::dot(n, triangle1 - segment1) / glm::dot(n, u);
	if (depth > glm::length(u) || depth < 0.f) return false; // too far or beind
	glm::vec3 intersection = segment1 + depth*u - triangle1;

	//	checking barycentric coordinates
	float crossDot = glm::dot(v1, v2);
	float magnitute = glm::dot(v1, v1)*glm::dot(v2, v2) - crossDot*crossDot;
	glm::vec2 barry;

	barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - crossDot * glm::dot(intersection, v2)) / magnitute;
	barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - crossDot * glm::dot(intersection, v1)) / magnitute;
	return !(barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f);
}
bool Collision::collide_SegmentvsOrientedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	//http://www.opengl-tutorial.org/fr/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/

	if (segment2 == segment1) return collide_PointvsOrientedBox(segment1, boxTranform, boxMin, boxMax);
	glm::vec3 u = glm::normalize(segment2 - segment1);
	glm::vec3 delta = glm::vec3(boxTranform[3]) - segment1;

	float tmin = 0.f;
	float tmax = std::numeric_limits<float>::max();

	//	test on the two axis of local x
	glm::vec3 bx = glm::vec3(boxTranform[0]);
	float e = glm::dot(bx, delta);
	if (glm::dot(bx, u) == 0.f) // segment parallel to selected plane
	{
		if (-e + boxMin.x > 0.0f || -e + boxMax.x < 0.0f) return false;
	}
	else
	{
		float t1 = (e + boxMin.x) / glm::dot(bx, u); // Intersection with the "left" plane
		float t2 = (e + boxMax.x) / glm::dot(bx, u); // Intersection with the "right" plane
		if (t1 > t2) std::swap(t1, t2);

		if (t2 < tmax) tmax = t2;
		if (t1 > tmin) tmin = t1;
		if (tmax < tmin) return false;
	}

	//	test on the two axis of local y
	glm::vec3 by = glm::vec3(boxTranform[1]);
	e = glm::dot(by, delta);
	if (glm::dot(by, u) == 0.f) // segment parallel to selected plane
	{
		if (-e + boxMin.y > 0.0f || -e + boxMax.y < 0.0f) return false;
	}
	else
	{
		float t1 = (e + boxMin.y) / glm::dot(by, u); // Intersection with the "left" plane
		float t2 = (e + boxMax.y) / glm::dot(by, u); // Intersection with the "right" plane
		if (t1 > t2) std::swap(t1, t2);

		if (t2 < tmax) tmax = t2;
		if (t1 > tmin) tmin = t1;
		if (tmax < tmin) return false;
	}

	//	test on the two axis of local z
	glm::vec3 bz = glm::vec3(boxTranform[2]);
	e = glm::dot(bz, delta);
	if (glm::dot(bz, u) == 0.f) // segment parallel to selected plane
	{
		if (-e + boxMin.z > 0.0f || -e + boxMax.z < 0.0f) return false;
	}
	else
	{
		float t1 = (e + boxMin.z) / glm::dot(bz, u); // Intersection with the "left" plane
		float t2 = (e + boxMax.z) / glm::dot(bz, u); // Intersection with the "right" plane
		if (t1 > t2) std::swap(t1, t2);

		if (t2 < tmax) tmax = t2;
		if (t1 > tmin) tmin = t1;
		if (tmax < tmin) return false;
	}
	return tmin <= glm::length(segment2 - segment1);
}
bool Collision::collide_SegmentvsAxisAlignedBox(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	if (segment2 == segment1) return collide_PointvsAxisAlignedBox(segment1, boxMin, boxMax);

	glm::vec3 s = segment2 - segment1;
	glm::vec3 u = glm::normalize(s);
	glm::vec3 t1 = (boxMin - segment1) / u;
	glm::vec3 t2 = (boxMax - segment1) / u;
	float tnear = glm::compMax(glm::min(t1, t2));
	float tfar = glm::compMin(glm::max(t1, t2));
	if (tfar >= tnear && tfar >= 0 && tnear <= glm::length(s)) return true;
	else return false;
}
bool Collision::collide_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (segment2 == segment1) return collide_PointvsSphere(segment1, sphereCenter, sphereRadius);

	glm::vec3 s = segment2 - segment1;
	glm::vec3 u = glm::normalize(s);
	glm::vec3 u2 = sphereCenter - segment1;
	glm::vec3 u3 = u2 - glm::dot(u, u2) * u; // distance (actualy a vector) of sphere center to ray
	return glm::dot(u3, u3) <= sphereRadius*sphereRadius && glm::dot(u, u2) <= glm::length(s) && glm::dot(u, u2) >= 0.f;
}
bool Collision::collide_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	glm::vec3 s1 = segment2 - segment1;
	glm::vec3 s2 = capsule2 - capsule1;
	glm::vec3 n = glm::cross(s1, s2);

	if (n == glm::vec3(0.f))	// parallel or one segment is a point
	{
		if (s1 == glm::vec3(0.f))
			return collide_PointvsCapsule(segment1, capsule1, capsule2, capsuleRadius);
		else if (s2 == glm::vec3(0.f))
			return collide_SegmentvsSphere(segment1, segment2, capsule1, capsuleRadius);
		else // segment are parallel
		{
			glm::vec3 u1 = glm::normalize(s1);
			glm::vec3 u3 = segment1 - capsule1;
			glm::vec3 d = u3 - u1 * std::abs(glm::dot(u3, u1));
			return glm::length(d) <= std::max(capsuleRadius, EPSILON);
		}
	}
	else
	{
		glm::vec3 u1 = glm::normalize(s1);
		glm::vec3 u2 = glm::normalize(s2);
		n = glm::normalize(n);
		float t1 = -glm::determinant(glm::mat3(segment1 - capsule1, u2, n)) / glm::dot(n, n);
		float t2 = -glm::determinant(glm::mat3(segment1 - capsule1, u1, n)) / glm::dot(n, n);

		glm::vec3 d = capsule1 + u2*t2 - (segment1 + u1*t1);
		return glm::length(d) <= std::max(capsuleRadius, EPSILON);
	}
}
//

//	Specialized functions : triangle
bool Collision::collide_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c)
{
	return false;
}
bool Collision::collide_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return false;
}
bool Collision::collide_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return false;
}
bool Collision::collide_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return false;
}
bool Collision::collide_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : oriented box
bool Collision::collide_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	//	axis to check in absolute base
	glm::vec3 xb1 = glm::vec3(box1Tranform*glm::vec4(1.f, 0.f, 0.f, 0.f));
	glm::vec3 yb1 = glm::vec3(box1Tranform*glm::vec4(0.f, 1.f, 0.f, 0.f));
	glm::vec3 zb1 = glm::vec3(box1Tranform*glm::vec4(0.f, 0.f, 1.f, 0.f));
	glm::vec3 xb2 = glm::vec3(box2Tranform*glm::vec4(1.f, 0.f, 0.f, 0.f));
	glm::vec3 yb2 = glm::vec3(box2Tranform*glm::vec4(0.f, 1.f, 0.f, 0.f));
	glm::vec3 zb2 = glm::vec3(box2Tranform*glm::vec4(0.f, 0.f, 1.f, 0.f));

	//	box position and halfSize
	glm::vec3 distance = 0.5f*glm::vec3(box1Tranform*glm::vec4(box1Min + box1Max, 1.f)) - 0.5f*glm::vec3(box1Tranform*glm::vec4(box2Min + box2Max, 1.f));
	glm::vec3 sb1 = box1Max - box1Min;
	glm::vec3 sb2 = box2Max - box2Min;

	//	first test pass
	if      (std::abs(glm::dot(xb1, distance)) > projectBox(xb1, sb1) + projectBox(xb1, sb2)) return false;
	else if (std::abs(glm::dot(yb1, distance)) > projectBox(yb1, sb1) + projectBox(yb1, sb2)) return false;
	else if (std::abs(glm::dot(zb1, distance)) > projectBox(zb1, sb1) + projectBox(zb1, sb2)) return false;
	else if (std::abs(glm::dot(xb2, distance)) > projectBox(xb2, sb1) + projectBox(xb2, sb2)) return false;
	else if (std::abs(glm::dot(yb2, distance)) > projectBox(yb2, sb1) + projectBox(yb2, sb2)) return false;
	else if (std::abs(glm::dot(zb2, distance)) > projectBox(zb2, sb1) + projectBox(zb2, sb2)) return false;
	
	//	secondary axis checking
	glm::vec3 xb1xb2 = glm::normalize(glm::cross(xb1, xb2));
	glm::vec3 xb1yb2 = glm::normalize(glm::cross(xb1, yb2));
	glm::vec3 xb1zb2 = glm::normalize(glm::cross(xb1, zb2));
	glm::vec3 yb1xb2 = glm::normalize(glm::cross(yb1, xb2));
	glm::vec3 yb1yb2 = glm::normalize(glm::cross(yb1, yb2));
	glm::vec3 yb1zb2 = glm::normalize(glm::cross(yb1, zb2));
	glm::vec3 zb1xb2 = glm::normalize(glm::cross(zb1, xb2));
	glm::vec3 zb1yb2 = glm::normalize(glm::cross(zb1, yb2));
	glm::vec3 zb1zb2 = glm::normalize(glm::cross(zb1, zb2));

	//	second test pass
	if      (std::abs(glm::dot(xb1xb2, distance)) > projectBox(xb1xb2, sb1) + projectBox(xb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(xb1yb2, distance)) > projectBox(xb1yb2, sb1) + projectBox(xb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(xb1zb2, distance)) > projectBox(xb1zb2, sb1) + projectBox(xb1zb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1xb2, distance)) > projectBox(yb1xb2, sb1) + projectBox(yb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1yb2, distance)) > projectBox(yb1yb2, sb1) + projectBox(yb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(yb1zb2, distance)) > projectBox(yb1zb2, sb1) + projectBox(yb1zb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1xb2, distance)) > projectBox(zb1xb2, sb1) + projectBox(zb1xb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1yb2, distance)) > projectBox(zb1yb2, sb1) + projectBox(zb1yb2, sb2)) return false;
	else if (std::abs(glm::dot(zb1zb2, distance)) > projectBox(zb1zb2, sb1) + projectBox(zb1zb2, sb2)) return false;
	else return true;
}
bool Collision::collide_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return collide_OrientedBoxvsOrientedBox(box1Tranform, box1Min, box1Max, glm::mat4(1.f), box2Min, box2Max);
}
bool Collision::collide_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	//	for help read https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
	//	line 165

	glm::vec3 bcenter = 0.5f * (glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) + glm::vec3(boxTranform*glm::vec4(boxMin, 1.f)));
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 bx = glm::normalize(glm::vec3(boxTranform*glm::vec4(1.f, 0.f, 0.f, 0.f)));	// box local x
	glm::vec3 by = glm::normalize(glm::vec3(boxTranform*glm::vec4(0.f, 1.f, 0.f, 0.f)));	// box local y
	glm::vec3 bz = glm::normalize(glm::vec3(boxTranform*glm::vec4(0.f, 0.f, 1.f, 0.f)));	// box local z

	glm::vec3 p = sphereCenter - bcenter;
	glm::vec3 boxClosestPoint = bcenter;

	float d = glm::dot(bx, p);
	if (d > bsize.x) d = bsize.x;
	else if (d < -bsize.x) d = -bsize.x;
	boxClosestPoint += d* bx;

	d = glm::dot(by, p);
	if (d > bsize.y) d = bsize.y;
	else if (d < -bsize.y) d = -bsize.y;
	boxClosestPoint += d* by;

	d = glm::dot(bz, p);
	if (d > bsize.z) d = bsize.z;
	else if (d < -bsize.z) d = -bsize.z;
	boxClosestPoint += d* bz;

	return collide_PointvsSphere(boxClosestPoint, sphereCenter, sphereRadius);
}
bool Collision::collide_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : axis aligned box
bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return box1Min.x <= box2Max.x && box1Min.y <= box2Max.y && box1Min.z <= box2Max.z && box2Min.x <= box1Max.x && box2Min.y <= box1Max.y && box2Min.z <= box1Max.z;
}
bool Collision::collide_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	//	special case of obb/sphere
	glm::vec3 bcenter = 0.5f * (boxMax + boxMin);
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 p = sphereCenter - bcenter;

	if (p.x > bsize.x) p.x = bsize.x;
	else if (p.x < -bsize.x) p.x = -bsize.x;
	if (p.y > bsize.y) p.y = bsize.y;
	else if (p.y < -bsize.y) p.y = -bsize.y;
	if (p.z > bsize.z) p.z = bsize.z;
	else if (p.z < -bsize.z) p.z = -bsize.z;

	glm::vec3 boxClosestPoint = sphereCenter + p;
	return collide_PointvsSphere(boxClosestPoint, sphereCenter, sphereRadius);
}
inline bool Collision::collide_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : sphere
bool Collision::collide_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius)
{
	return glm::length(sphere2Center - sphere1Center) <= sphere1Radius + sphere2Radius;
}
bool Collision::collide_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	glm::vec3 u1 = capsule2 - capsule1;
	glm::vec3 u2 = sphereCenter - capsule1;
	glm::vec3 I = capsule1 + u1 * std::min(1.f, std::max(0.f, glm::dot(u2, u1)));
	return glm::length(sphereCenter - I) <= sphereRadius + capsuleRadius;
}
//

//	Specialized functions : capsule
bool Collision::collide_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	glm::vec3 s1 = capsule1b - capsule1a;
	glm::vec3 s2 = capsule2b - capsule2a;
	glm::vec3 u1 = glm::normalize(s1);
	glm::vec3 u2 = glm::normalize(s2);
	glm::vec3 n = glm::cross(u1, u2);

	if (glm::dot(n, n) == 0.f)	// parallel or one segment is a point
	{
		if (u1 == glm::vec3(0.f))
			return collide_SpherevsCapsule(capsule1b, capsule1Radius, capsule2a, capsule2b, capsule2Radius);
		else if (u2 == glm::vec3(0.f))
			return collide_SpherevsCapsule(capsule2b, capsule2Radius, capsule1a, capsule1b, capsule1Radius);
		else // segment are parallel
		{
			glm::vec3 u3 = capsule1a - capsule2a;
			glm::vec3 d = u3 - u1 * std::abs(glm::dot(u3, u1));
			return glm::length(d) <= capsule1Radius + capsule2Radius;
		}
	}
	else
	{
		float t1 = std::min(glm::length(s1), std::max(0.f, glm::determinant(glm::mat3(capsule1a - capsule2a, u2, n)) / glm::dot(n, n)));
		float t2 = std::min(glm::length(s2), std::max(0.f, glm::determinant(glm::mat3(capsule1a - capsule2a, u1, n)) / glm::dot(n, n)));
		glm::vec3 d = capsule2a + u2*t2 - (capsule1a + u1*t1);
		return glm::length(d) <= capsule1Radius + capsule2Radius;
	}
}
//

