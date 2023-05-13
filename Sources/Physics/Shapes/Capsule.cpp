#include "Capsule.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/


Capsule::Capsule(const vec4f& a, const vec4f& b, const float& r) : Shape(ShapeType::CAPSULE), p1(a), p2(b), radius(r) {}
Sphere Capsule::toSphere() const { return Sphere(0.5f * (p1 + p2), radius + 0.5f * (p1 - p2).getNorm()); }
AxisAlignedBox Capsule::toAxisAlignedBox() const
{
	const vec4f r = vec4f(radius, radius, radius, 0);
	return AxisAlignedBox(vec4f::min(p1, p2) - r, vec4f::max(p1, p2) + r);
}
Shape& Capsule::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::CAPSULE)
	{
		const Capsule& capsule = *static_cast<const Capsule*>(&s);
		p1 = capsule.p1;
		p2 = capsule.p2;
		radius = capsule.radius;
	}
	return *this;
}
void Capsule::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	/*vec4f::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);*/

	p1 = m * p1;
	p2 = m * p2;
	radius = radius * std::max(scale.x, std::max(scale.y, scale.z));
}
Shape* Capsule::duplicate() const { return new Capsule(*this); }
vec4f Capsule::support(const vec4f& direction) const
{
	if (vec4f::dot(p1, direction) > vec4f::dot(p2, direction))
		return p1 + direction.getNormal() * radius;
	else return p2 + direction.getNormal() * radius;
}
void Capsule::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	vec4f v =(p2 - p1).getNormal();
	if (std::abs(vec4f::dot(v, direction)) < 0.1f)
	{
		points.push_back(p1 + direction * radius);
		points.push_back(p2 + direction * radius);
	}
	else
		points.push_back(support(direction));
}
mat4f Capsule::computeInertiaMatrix() const
{
	/*
	            ^ z
	            |
			   ___
	          /   \
	         |     |
	         |  *  |
	         |     |
	          \___/   --> x
			/
		   /y
	*/

	float height = (p2 - p1).getNorm();
	mat4f M(0.f);
	M[0][0] = M[1][1] = 0.25f * radius * radius + 0.333f * height * height + 3.f / 8.f * radius * height;
	M[2][2] = 0.9f * radius * radius;
	M[3][3] = 1.f;

	mat4f T = mat4f::fromTo(vec4f(0, 0, 1, 0), (p2 - p1).getNormal());
	mat4f iT = mat4f::transpose(T);
	return T * M * iT;
}