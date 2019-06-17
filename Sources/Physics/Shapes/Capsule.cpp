#include "Capsule.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>


Capsule::Capsule(const glm::vec3& a, const glm::vec3& b, const float& r) : Shape(CAPSULE), p1(a), p2(b), radius(r) {}
Sphere Capsule::toSphere() const { return Sphere(0.5f*(p1 + p2), radius + 0.5f*glm::length(p1 - p2)); }
AxisAlignedBox Capsule::toAxisAlignedBox() const
{
	return AxisAlignedBox(glm::min(p1, p2) - glm::vec3(radius), glm::max(p1, p2) + glm::vec3(radius));
}
Shape& Capsule::operator=(const Shape& s)
{
	if (s.type == Shape::CAPSULE)
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
glm::vec3 Capsule::GJKsupport(const glm::vec3& direction) const
{
	if (glm::dot(p1, direction) > glm::dot(p2, direction))
		return p1 + glm::normalize(direction) * radius;
	else return p2 + glm::normalize(direction) * radius;
}