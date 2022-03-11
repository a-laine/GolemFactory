#include "Triangle.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Triangle::Triangle(const glm::vec4& a, const glm::vec4& b, const glm::vec4& c) : Shape(ShapeType::TRIANGLE), p1(a), p2(b), p3(c) {}
Sphere Triangle::toSphere() const
{
	glm::vec4 u1 = p2 - p1;
	glm::vec4 u2 = p3 - p1;
	glm::vec4 n = glm::vec4(glm::cross((glm::vec3)u1, (glm::vec3)u2), 0);
	float nmag = glm::length(n);

	if (nmag == 0.f) // flat triangle
	{
		float d1 = glm::length(p2 - p1);
		float d2 = glm::length(p3 - p1);
		float d3 = glm::length(p2 - p3);
		if (d1 > d2 && d1 > d3) return Sphere(0.5f * (p1 + p2), 0.5f * d1);
		else if (d2 > d1 && d2 > d3) return Sphere(0.5f * (p1 + p3), 0.5f * d2);
		else return Sphere(0.5f * (p3 + p2), 0.5f * d3);
	}
	else
	{
		float radius = glm::length(u1)*glm::length(u2)*glm::length(u1 - u2) / nmag;
		glm::vec4 cross = glm::vec4(glm::cross((glm::vec3)(glm::dot(u1, u1) * u2 - glm::dot(u2, u2) * u1), (glm::vec3)n), 0);
		glm::vec4 center = p1 + cross / (2.f * nmag * nmag);
		return Sphere(center, radius);
	}
}
AxisAlignedBox Triangle::toAxisAlignedBox() const
{
	return AxisAlignedBox(glm::min(p1, glm::min(p2, p3)), glm::max(p1, glm::max(p2, p3)));
}
Shape& Triangle::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::TRIANGLE)
	{
		const Triangle& triangle = *static_cast<const Triangle*>(&s);
		p1 = triangle.p1;
		p2 = triangle.p2;
		p3 = triangle.p3;
	}
	return *this;
}
void Triangle::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), (glm::vec3)position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p1 = m * p1;
	p2 = m * p2;
	p3 = m * p3;
}
Shape* Triangle::duplicate() const { return new Triangle(*this); }
glm::vec4 Triangle::support(const glm::vec4& direction) const
{
	float a = glm::dot(p1, direction);
	float b = glm::dot(p2, direction);
	float c = glm::dot(p3, direction);

	if (a > b && a > c) return p1;
	else if (b > a && b > c) return p2;
	else return p3;
}
void Triangle::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	points.push_back(p1);
	points.push_back(p2);
	points.push_back(p3);
}
