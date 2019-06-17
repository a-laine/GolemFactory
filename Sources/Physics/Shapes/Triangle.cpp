#include "Triangle.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

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
		if (d1 > d2 && d1 > d3) return Sphere(0.5f*(p1 + p2), 0.5f*d1);
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
AxisAlignedBox Triangle::toAxisAlignedBox() const
{
	return AxisAlignedBox(glm::min(p1, glm::min(p2, p3)), glm::max(p1, glm::max(p2, p3)));
}
Shape& Triangle::operator=(const Shape& s)
{
	if (s.type == Shape::TRIANGLE)
	{
		const Triangle& triangle = *static_cast<const Triangle*>(&s);
		p1 = triangle.p1;
		p2 = triangle.p2;
		p3 = triangle.p3;
	}
	return *this;
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
glm::vec3 Triangle::GJKsupport(const glm::vec3& direction) const
{
	float a = glm::dot(p1, direction);
	float b = glm::dot(p2, direction);
	float c = glm::dot(p3, direction);

	if (a > b && a > c) return p1;
	else if (b > a && b > c) return p2;
	else return p3;
}
