#include "Triangle.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/

Triangle::Triangle(const vec4f& a, const vec4f& b, const vec4f& c) : Shape(ShapeType::TRIANGLE), p1(a), p2(b), p3(c) {}
Sphere Triangle::toSphere() const
{
	vec4f u1 = p2 - p1;
	vec4f u2 = p3 - p1;
	vec4f n = vec4f::cross(u1, u2);
	float nmag = n.getNorm();

	if (nmag == 0.f) // flat triangle
	{
		float d1 = (p2 - p1).getNorm();
		float d2 = (p3 - p1).getNorm();
		float d3 = (p2 - p3).getNorm();
		if (d1 > d2 && d1 > d3) return Sphere(0.5f * (p1 + p2), 0.5f * d1);
		else if (d2 > d1 && d2 > d3) return Sphere(0.5f * (p1 + p3), 0.5f * d2);
		else return Sphere(0.5f * (p3 + p2), 0.5f * d3);
	}
	else
	{
		float radius = u1.getNorm() *u2.getNorm() *(u1 - u2).getNorm() / nmag;
		vec4f cross = vec4f::cross(vec4f::dot(u1, u1) * u2 - vec4f::dot(u2, u2) * u1, n);
		vec4f center = p1 + cross / (2.f * nmag * nmag);
		return Sphere(center, radius);
	}
}
AxisAlignedBox Triangle::toAxisAlignedBox() const
{
	return AxisAlignedBox(vec4f::min(p1, vec4f::min(p2, p3)), vec4f::max(p1, vec4f::max(p2, p3)));
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
void Triangle::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	p1 = m * p1;
	p2 = m * p2;
	p3 = m * p3;
}
Shape* Triangle::duplicate() const { return new Triangle(*this); }
vec4f Triangle::support(const vec4f& direction) const
{
	float a = vec4f::dot(p1, direction);
	float b = vec4f::dot(p2, direction);
	float c = vec4f::dot(p3, direction);

	if (a > b && a > c) return p1;
	else if (b > a && b > c) return p2;
	else return p3;
}
void Triangle::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	points.push_back(p1);
	points.push_back(p2);
	points.push_back(p3);
}
