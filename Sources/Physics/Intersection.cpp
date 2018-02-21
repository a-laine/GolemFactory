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
	glm::vec3 bmin = glm::vec3(boxTranform*glm::vec4(boxMin, 1.f));
	glm::vec3 bdiag = glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) - bmin;
	glm::vec3 bx = glm::normalize(glm::vec3(boxTranform*glm::vec4(1.f, 0.f, 0.f, 0.f)));
	glm::vec3 by = glm::normalize(glm::vec3(boxTranform*glm::vec4(0.f, 1.f, 0.f, 0.f)));
	glm::vec3 bz = glm::normalize(glm::vec3(boxTranform*glm::vec4(0.f, 0.f, 1.f, 0.f)));

	glm::vec3 p = point - bmin;
	if (glm::dot(p, bx) < 0.f || glm::dot(p, by) < 0.f || glm::dot(p, bz) < 0.f) return false;
	else if (glm::dot(p, bx) > glm::dot(bdiag, bx) || glm::dot(p, by) > glm::dot(bdiag, by) || glm::dot(p, bz) > glm::dot(bdiag, bz)) return false;
	else return true;
}
inline bool Intersection::intersect_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	if(point.x < boxMin.x || point.y < boxMin.y || point.z < boxMin.z) return false;
	else if (point.x > boxMax.x || point.y > boxMax.y || point.z > boxMax.z) return false;
	else return true;
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

//	Specialized functions : segment
inline bool Intersection::intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b)
{
	glm::vec3 s1 = segment1b - segment1a;
	glm::vec3 s2 = segment2b - segment2a;
	glm::vec3 u1 = glm::normalize(s1);
	glm::vec3 u2 = glm::normalize(s2);
	glm::vec3 n = glm::cross(u1, u2);

	if (glm::dot(n, n) == 0.f)	// parallel or one segment is a point
	{
		if (u1 == glm::vec3(0.f))
			return intersect_PointvsSegment(segment1b, segment2a, segment2b);
		else if (u2 == glm::vec3(0.f))
			return intersect_PointvsSegment(segment2b, segment2a, segment2b);
		else // segment are parallel
		{
			glm::vec3 u3 = segment1a - segment2a;
			glm::vec3 d = u3 - u1 * std::abs(glm::dot(u3, u1));
			return glm::dot(d, d) == 0.f;
		}
	}
	else
	{
		float t1 = std::min(glm::length(s1), std::max(0.f, glm::determinant(glm::mat3(segment1a - segment2a, u2, n)) / glm::dot(n, n)));
		float t2 = std::min(glm::length(s2), std::max(0.f, glm::determinant(glm::mat3(segment1a - segment2a, u1, n)) / glm::dot(n, n)));
		glm::vec3 d = segment2a + u2*t2 - (segment1a + u1*t1);
		return glm::dot(d, d) == 0.f;
	}
}
inline bool Intersection::intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	return false;
}
inline bool Intersection::intersect_SegmentvsOrientedBox(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return false;
}
inline bool Intersection::intersect_SegmentvsAxisAlignedBox(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return false;
}
inline bool Intersection::intersect_SegmentvsSphere(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return false;
}
inline bool Intersection::intersect_SegmentvsCapsule(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : triangle
inline bool Intersection::intersect_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c)
{
	return false;
}
inline bool Intersection::intersect_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return false;
}
inline bool Intersection::intersect_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return false;
}
inline bool Intersection::intersect_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return false;
}
inline bool Intersection::intersect_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : oriented box
inline bool Intersection::intersect_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return false;
}
inline bool Intersection::intersect_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return false;
}
inline bool Intersection::intersect_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return false;
}
inline bool Intersection::intersect_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : axis aligned box
inline bool Intersection::intersect_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max)
{
	return box1Min.x <= box2Max.x && box1Min.y <= box2Max.y && box1Min.z <= box2Max.z && box2Min.x <= box1Max.x && box2Min.y <= box1Max.y && box2Min.z <= box1Max.z;
}
inline bool Intersection::intersect_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return false;
}
inline bool Intersection::intersect_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return false;
}
//

//	Specialized functions : sphere
inline bool Intersection::intersect_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius)
{
	return glm::length(sphere2Center - sphere1Center) <= sphere1Radius + sphere2Radius;
}
inline bool Intersection::intersect_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	glm::vec3 u1 = capsule2 - capsule1;
	glm::vec3 u2 = sphereCenter - capsule1;
	glm::vec3 I = capsule1 + u1 * std::min(1.f, std::max(0.f, glm::dot(u2, u1)));
	return glm::length(sphereCenter - I) <= sphereRadius + capsuleRadius;
}
//

//	Specialized functions : capsule
inline bool Intersection::intersect_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius)
{
	//	see http://www.realtimerendering.com/intersections.html#I304 : ray/ray algorithm
	glm::vec3 s1 = capsule1b - capsule1a;
	glm::vec3 s2 = capsule2b - capsule2a;
	glm::vec3 u1 = glm::normalize(s1);
	glm::vec3 u2 = glm::normalize(s2);
	glm::vec3 n = glm::cross(u1, u2);

	if (glm::dot(n, n) == 0.f)	// parallel or one segment is a point
	{
		if (u1 == glm::vec3(0.f))
			return intersect_SpherevsCapsule(capsule1b, capsule1Radius, capsule2a, capsule2b, capsule2Radius);
		else if (u2 == glm::vec3(0.f))
			return intersect_SpherevsCapsule(capsule2b, capsule2Radius, capsule1a, capsule1b, capsule1Radius);
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

