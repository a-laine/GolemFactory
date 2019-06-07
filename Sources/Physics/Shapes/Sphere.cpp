#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>


Sphere::Sphere(const glm::vec3& position, const float& r) : Shape(SPHERE), center(position), radius(r) {}
Sphere Sphere::toSphere() const { return *this; }
AxisAlignedBox Sphere::toAxisAlignedBox() const
{
	return AxisAlignedBox(center - glm::vec3(radius), center + glm::vec3(radius));
}
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
glm::vec3 Sphere::GJKsupport(const glm::vec3& direction) const { return center + glm::normalize(direction) * radius; }