#include "Intersection.h"

//	Private field
namespace
{
	inline bool intersect_PointvsShape(const Shape& point, const Shape& b)
	{
		const Point* a = reinterpret_cast<const Point*>(&point);
		switch (b.type)
		{
			case Shape::POINT: {
				const Point* c = reinterpret_cast<const Point*>(&b);
				return Intersection::intersect_PointvsPoint(a->p, c->p);
			}
			case Shape::SEGMENT: {
				const Segment* c = reinterpret_cast<const Segment*>(&b);
				return Intersection::intersect_PointvsSegment(a->p, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = reinterpret_cast<const Triangle*>(&b);
				return Intersection::intersect_PointvsTriangle(a->p, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Intersection::intersect_PointvsOrientedBox(a->p, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_PointvsAxisAlignedBox(a->p, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Intersection::intersect_PointvsSphere(a->p, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Intersection::intersect_PointvsCapsule(a->p, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool intersect_SegmentvsShape(const Shape& segment, const Shape& b)
	{
		const Segment* a = reinterpret_cast<const Segment*>(&segment);
		switch (b.type)
		{
			case Shape::SEGMENT: {
				const Segment* c = reinterpret_cast<const Segment*>(&b);
				return Intersection::intersect_SegmentvsSegment(a->p1, a->p2, c->p1, c->p2);
			}
			case Shape::TRIANGLE: {
				const Triangle* c = reinterpret_cast<const Triangle*>(&b);
				return Intersection::intersect_SegmentvsTriangle(a->p1, a->p2, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Intersection::intersect_SegmentvsOrientedBox(a->p1, a->p2, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_SegmentvsAxisAlignedBox(a->p1, a->p2, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Intersection::intersect_SegmentvsSphere(a->p1, a->p2, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Intersection::intersect_SegmentvsCapsule(a->p1, a->p2, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool intersect_TrianglevsShape(const Shape& triangle, const Shape& b)
	{
		const Triangle* a = reinterpret_cast<const Triangle*>(&triangle);
		switch (b.type)
		{
			case Shape::TRIANGLE: {
				const Triangle* c = reinterpret_cast<const Triangle*>(&b);
				return Intersection::intersect_TrianglevsTriangle(a->p1, a->p2, a->p3, c->p1, c->p2, c->p3);
			}
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Intersection::intersect_TrianglevsOrientedBox(a->p1, a->p2, a->p3, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_TrianglevsAxisAlignedBox(a->p1, a->p2, a->p3, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Intersection::intersect_TrianglevsSphere(a->p1, a->p2, a->p3, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Intersection::intersect_TrianglevsCapsule(a->p1, a->p2, a->p3, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool intersect_OrientedBoxvsShape(const Shape& obox, const Shape& b)
	{
		const OrientedBox* a = reinterpret_cast<const OrientedBox*>(&obox);
		switch (b.type)
		{
			case Shape::ORIENTED_BOX: {
				const OrientedBox* c = reinterpret_cast<const OrientedBox*>(&b);
				return Intersection::intersect_OrientedBoxvsOrientedBox(a->transform, a->min, a->max, c->transform, c->min, c->max);
			}
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_OrientedBoxvsAxisAlignedBox(a->transform, a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Intersection::intersect_OrientedBoxvsSphere(a->transform, a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Intersection::intersect_OrientedBoxvsCapsule(a->transform, a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool intersect_AxisAlignedBoxvsShape(const Shape& aabox, const Shape& b)
	{
		const AxisAlignedBox* a = reinterpret_cast<const AxisAlignedBox*>(&aabox);
		switch (b.type)
		{
			case Shape::AXIS_ALIGNED_BOX: {
				const AxisAlignedBox* c = reinterpret_cast<const AxisAlignedBox*>(&b);
				return Intersection::intersect_AxisAlignedBoxvsAxisAlignedBox(a->min, a->max, c->min, c->max);
			}
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Intersection::intersect_AxisAlignedBoxvsSphere(a->min, a->max, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Intersection::intersect_AxisAlignedBoxvsCapsule(a->min, a->max, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool intersect_SpherevsShape(const Shape& sphere, const Shape& b)
	{
		const Sphere* a = reinterpret_cast<const Sphere*>(&sphere);
		switch (b.type)
		{
			case Shape::SPHERE: {
				const Sphere* c = reinterpret_cast<const Sphere*>(&b);
				return Intersection::intersect_SpherevsSphere(a->center, a->radius, c->center, c->radius);
			}
			case Shape::CAPSULE: {
				const Capsule* c = reinterpret_cast<const Capsule*>(&b);
				return Intersection::intersect_SpherevsCapsule(a->center, a->radius, c->p1, c->p2, c->radius);
			}
			default: return false;
		}
	};
	inline bool intersect_CapsulevsShape(const Shape& capsule, const Shape& b)
	{
		const Capsule* a = reinterpret_cast<const Capsule*>(&capsule);
		if(b.type == Shape::CAPSULE)
		{
			const Capsule* c = reinterpret_cast<const Capsule*>(&b);
			return Intersection::intersect_CapsulevsCapsule(a->p1, a->p2, a->radius, c->p1, c->p2, c->radius);
		}
		else return false;
	};
}
//

//	Public field
bool Intersection::intersect(const Shape& a, const Shape& b)
{
	//	order objects
	Shape& shape1 = (Shape&) a;
	Shape& shape2 = (Shape&) b;
	if (a.type < b.type) std::swap(shape1, shape2);

	switch (shape1.type)
	{
		case Shape::POINT: return intersect_PointvsShape(shape1, shape2);
		case Shape::SEGMENT: return intersect_SegmentvsShape(shape1, shape2);
		case Shape::TRIANGLE: return intersect_TrianglevsShape(shape1, shape2);
		case Shape::ORIENTED_BOX: return intersect_OrientedBoxvsShape(shape1, shape2);
		case Shape::AXIS_ALIGNED_BOX: return intersect_AxisAlignedBoxvsShape(shape1, shape2);
		case Shape::SPHERE: return intersect_SpherevsShape(shape1, shape2);
		case Shape::CAPSULE: return intersect_CapsulevsShape(shape1, shape2);
		default: return false;
	}
}
//

//	Specialized functions : point
inline bool Intersection::intersect_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2)
{
	return point1 == point2;
}
inline bool Intersection::intersect_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2)
{
	glm::vec3 u1 = segment2 - segment1;
	glm::vec3 u2 = point - segment1;
	return glm::length(glm::cross(u1, u2)) == 0.f && glm::dot(u1, u2) >= 0.f && glm::dot(u1, u2) <= glm::length(u1);
}
inline bool Intersection::intersect_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	//	check if point is coplanar to triangle
	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(u1, u2);
	glm::vec3 p = point - triangle1;

	if (glm::dot(p, n) == 0.f && n != glm::vec3(0.f))
	{
		glm::normalize(n);

		//	checking barycentric coordinates
		float magnitute = glm::dot(u1, u1)*glm::dot(u2, u2) - glm::dot(u1, u2)*glm::dot(u1, u2);
		glm::vec2 barry;
		barry.x = (glm::dot(u2, u2) * glm::dot(p, u1) - glm::dot(u2, u1) * glm::dot(p, u2)) / magnitute;
		barry.y = (glm::dot(u1, u1) * glm::dot(p, u2) - glm::dot(u2, u1) * glm::dot(p, u1)) / magnitute;
		return !(barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f);
	}
	else return false;
}
inline bool Intersection::intersect_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{

}
inline bool Intersection::intersect_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
{

}
inline bool Intersection::intersect_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	glm::vec3 u = point - sphereCenter;
	return glm::length(u) <= sphereRadius;
}
inline bool Intersection::intersect_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	float d1 = glm::length(point - capsule1);
	glm::vec3 u1 = capsule2 - capsule1;

	if (glm::length(u1) == 0.f) return d1 <= capsuleRadius;
	else
	{
		glm::vec3 u2 = point - capsule1;
		float d2 = glm::length(point - capsule2);
		float d3 = glm::length(glm::cross(u2, u1)) / glm::length(u1);
		return d1 <= capsuleRadius && d2 <= capsuleRadius && d3 <= capsuleRadius;
	}
}
//