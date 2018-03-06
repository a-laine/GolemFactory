#include "Shape.h"

//	Default
Shape::Shape(const ShapeType& shapeType) : type(shapeType) {}
Sphere Shape::toSphere() const { return Sphere(glm::vec3(0.f), 0.f); }
//


//	Others shapes
Point::Point(const glm::vec3& position) : Shape(POINT), p(position){}
Sphere Point::toSphere() const { return Sphere(p, 0.f); }


Segment::Segment(const glm::vec3& a, const glm::vec3& b) : Shape(SEGMENT), p1(a), p2(b) {}
Sphere Segment::toSphere() const { return Sphere(0.5f*(p1+p2), 0.5f*glm::length(p1 - p2)); }


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


OrientedBox::OrientedBox(const glm::mat4& transformationMatrix, const glm::vec3& localMin, const glm::vec3& localMax)
	: Shape(ORIENTED_BOX), transform(transformationMatrix), min(localMin), max(localMax) {}
Sphere OrientedBox::toSphere() const
{
	glm::vec3 p1 = glm::vec3(transform*glm::vec4(min, 1.f));
	glm::vec3 p2 = glm::vec3(transform*glm::vec4(max, 1.f));
	return Sphere(0.5f*(p1 + p2), 0.5f*glm::length(p1 - p2));
}


AxisAlignedBox::AxisAlignedBox(const glm::vec3& cornerMin, const glm::vec3& cornerMax)
	: Shape(AXIS_ALIGNED_BOX), min(cornerMin), max(cornerMax) {}
Sphere AxisAlignedBox::toSphere() const { return Sphere(0.5f*(min + max), 0.5f*glm::length(min - max)); }


Sphere::Sphere(const glm::vec3& position, const float& r) : Shape(SPHERE), center(position), radius(r) {}
Sphere Sphere::toSphere() const { return *this; }


Capsule::Capsule(const glm::vec3& a, const glm::vec3& b, const float& r) : Shape(CAPSULE), p1(a), p2(b), radius(r) {}
Sphere Capsule::toSphere() const { return Sphere(0.5f*(p1 + p2), radius + 0.5f*glm::length(p1 - p2)); }
//

