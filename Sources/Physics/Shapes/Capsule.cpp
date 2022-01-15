#include "Capsule.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>


Capsule::Capsule(const glm::vec3& a, const glm::vec3& b, const float& r) : Shape(ShapeType::CAPSULE), p1(a), p2(b), radius(r) {}
Sphere Capsule::toSphere() const { return Sphere(0.5f*(p1 + p2), radius + 0.5f*glm::length(p1 - p2)); }
AxisAlignedBox Capsule::toAxisAlignedBox() const
{
	return AxisAlignedBox(glm::min(p1, p2) - glm::vec3(radius), glm::max(p1, p2) + glm::vec3(radius));
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
glm::vec3 Capsule::support(const glm::vec3& direction) const
{
	if (glm::dot(p1, direction) > glm::dot(p2, direction))
		return p1 + glm::normalize(direction) * radius;
	else return p2 + glm::normalize(direction) * radius;
}
void Capsule::getFacingFace(const glm::vec3& direction, std::vector<glm::vec3>& points) const
{
	glm::vec3 v = glm::normalize(p2 - p1);
	if (std::abs(glm::dot(v, direction)) < 0.1f)
	{
		points.push_back(p1 + direction * radius);
		points.push_back(p2 + direction * radius);
	}
	else
		points.push_back(support(direction));
}
glm::mat3 Capsule::computeInertiaMatrix() const
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

	float height = glm::length(p2 - p1);
	glm::mat3 M(0.f);
	M[0][0] = M[1][1] = 0.25f * radius * radius + 0.333f * height * height + 3.f / 8.f * radius * height;
	M[2][2] = 0.9f * radius * radius;

	glm::mat3 T = glm::mat3(glm::rotation(glm::vec3(0, 0, 1), glm::normalize(p2 - p1)));
	glm::mat3 iT = glm::transpose(T);
	return T * M * iT;
}