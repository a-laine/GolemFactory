#include "Shape.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>


//	Default
Intersection::Result::Result() : type1(Shape::NONE), type2(Shape::NONE), contact1(0.f), contact2(0.f), normal1(0.f), normal2(0.f)
{};

Shape::Shape(const ShapeType& shapeType) : type(shapeType) {}
Sphere Shape::toSphere() const { return Sphere(glm::vec3(0.f), 0.f); }
void Shape::operator=(const Shape& s) { GF_ASSERT(false); }
void Shape::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) {}
Shape* Shape::duplicate() const { return new Shape(type); }
//


//	Others shapes
Point::Point(const glm::vec3& position) : Shape(POINT), p(position){}
Sphere Point::toSphere() const { return Sphere(p, 0.f); }
void Point::operator=(const Shape& s)
{
	if (s.type == Shape::POINT)
	{
		const Point& point = *static_cast<const Point*>(&s);
		p = point.p;
	}
};
void Point::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) { p += position; }
Shape* Point::duplicate() const { return new Point(*this); }


Segment::Segment(const glm::vec3& a, const glm::vec3& b) : Shape(SEGMENT), p1(a), p2(b) {}
Sphere Segment::toSphere() const { return Sphere(0.5f*(p1+p2), 0.5f*glm::length(p1 - p2)); }
void Segment::operator=(const Shape& s)
{
	if (s.type == Shape::SEGMENT)
	{
		const Segment& segment = *static_cast<const Segment*>(&s);
		p1 = segment.p1;
		p2 = segment.p2;
	}
}
void Segment::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p1 = glm::vec3(m * glm::vec4(p1, 1.f));
	p2 = glm::vec3(m * glm::vec4(p2, 1.f));
}
Shape* Segment::duplicate() const { return new Segment(*this); }


Triangle::Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) : Shape(TRIANGLE), p1(a), p2(b), p3(c) {}
Sphere Triangle::toSphere() const
{
	glm::vec3 u1 = p2 - p1;
	glm::vec3 u2 = p3 - p1;
	glm::vec3 n = glm::cross(u1, u2);
	float nmag = glm::length(n);

	if (nmag == 0.f) // flat triangle
	{
		float d1 = glm::length(p2 - p1);
		float d2 = glm::length(p3 - p1);
		float d3 = glm::length(p2 - p3);
		if( d1 > d2 && d1 > d3) return Sphere(0.5f*(p1 + p2), 0.5f*d1);
		else if (d2 > d1 && d2 > d3) return Sphere(0.5f*(p1 + p3), 0.5f*d2);
		else return Sphere(0.5f*(p3 + p2), 0.5f*d3);
	}
	else
	{
		float radius = glm::length(u1)*glm::length(u2)*glm::length(u1 - u2) / nmag;
		glm::vec3 center = p1 + glm::cross((glm::dot(u1, u1)*u2 - glm::dot(u2, u2)*u1), n) / (2.f*nmag*nmag);
		return Sphere(center, radius);
	}
}
void Triangle::operator=(const Shape& s)
{
	if (s.type == Shape::TRIANGLE)
	{
		const Triangle& triangle = *static_cast<const Triangle*>(&s);
		p1 = triangle.p1;
		p2 = triangle.p2;
		p3 = triangle.p3;
	}
}
void Triangle::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p1 = glm::vec3(m * glm::vec4(p1, 1.f));
	p2 = glm::vec3(m * glm::vec4(p2, 1.f));
	p3 = glm::vec3(m * glm::vec4(p3, 1.f));
}
Shape* Triangle::duplicate() const { return new Triangle(*this); }


OrientedBox::OrientedBox(const glm::mat4& transformationMatrix, const glm::vec3& localMin, const glm::vec3& localMax)
	: Shape(ORIENTED_BOX), base(transformationMatrix), min(localMin), max(localMax) {}
Sphere OrientedBox::toSphere() const
{
	glm::vec3 p1 = glm::vec3(base*glm::vec4(min, 1.f));
	glm::vec3 p2 = glm::vec3(base*glm::vec4(max, 1.f));
	return Sphere(0.5f*(p1 + p2), 0.5f*glm::length(p2 - p1));
}
void OrientedBox::operator=(const Shape& s)
{
	if (s.type == Shape::ORIENTED_BOX)
	{
		const OrientedBox& box = *static_cast<const OrientedBox*>(&s);
		base = box.base;
		min = box.min;
		max = box.max;
	}
}
void OrientedBox::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	base = glm::translate(base, position);
	base = base * glm::toMat4(orientation);
	min = min * scale;
	max = max * scale;
}
Shape* OrientedBox::duplicate() const { return new OrientedBox(*this); }


AxisAlignedBox::AxisAlignedBox(const glm::vec3& cornerMin, const glm::vec3& cornerMax)
	: Shape(AXIS_ALIGNED_BOX), min(cornerMin), max(cornerMax) {}
Sphere AxisAlignedBox::toSphere() const { return Sphere(0.5f*(min + max), 0.5f*glm::length(min - max)); }
void AxisAlignedBox::operator=(const Shape& s)
{
	if (s.type == Shape::AXIS_ALIGNED_BOX)
	{
		const AxisAlignedBox& box = *static_cast<const AxisAlignedBox*>(&s);
		min = box.min;
		max = box.max;
	}
}
void AxisAlignedBox::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 base = glm::translate(glm::mat4(1.f), position);
	base = base * glm::toMat4(orientation);
	min = glm::vec3(base * glm::vec4(min, 1.f));
	max = glm::vec3(base * glm::vec4(max, 1.f));
	glm::vec3 M = glm::max(min * scale, max * scale);
	glm::vec3 m = glm::min(min * scale, max * scale);
	max = M; min = m;
}
Shape* AxisAlignedBox::duplicate() const { return new AxisAlignedBox(*this); }


Sphere::Sphere(const glm::vec3& position, const float& r) : Shape(SPHERE), center(position), radius(r) {}
Sphere Sphere::toSphere() const { return *this; }
void Sphere::operator=(const Shape& s)
{
	if (s.type == Shape::SPHERE)
	{
		const Sphere& sphere = *static_cast<const Sphere*>(&s);
		center = sphere.center;
		radius = sphere.radius;
	}
}
void Sphere::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	center = glm::vec3(m * glm::vec4(center, 1.f));
	radius = radius * glm::compMax(scale);
}
Shape* Sphere::duplicate() const { return new Sphere(*this); }


Capsule::Capsule(const glm::vec3& a, const glm::vec3& b, const float& r) : Shape(CAPSULE), p1(a), p2(b), radius(r) {}
Sphere Capsule::toSphere() const { return Sphere(0.5f*(p1 + p2), radius + 0.5f*glm::length(p1 - p2)); }
void Capsule::operator=(const Shape& s)
{
	if (s.type == Shape::CAPSULE)
	{
		const Capsule& capsule = *static_cast<const Capsule*>(&s);
		p1 = capsule.p1;
		p2 = capsule.p2;
		radius = capsule.radius;
	}
}
void Capsule::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p1 = glm::vec3(m * glm::vec4(p1, 1.f));
	p2 = glm::vec3(m * glm::vec4(p2, 1.f));
	radius = radius * glm::compMax(scale);
}
Shape* Capsule::duplicate() const { return new Capsule(*this); }
//

