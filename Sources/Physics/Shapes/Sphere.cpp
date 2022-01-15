#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>


Sphere::Sphere(const glm::vec3& position, const float& r) : Shape(ShapeType::SPHERE), center(position), radius(r) {}
Sphere Sphere::toSphere() const { return *this; }
AxisAlignedBox Sphere::toAxisAlignedBox() const
{
	return AxisAlignedBox(center - glm::vec3(radius), center + glm::vec3(radius));
}
Shape& Sphere::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::SPHERE)
	{
		const Sphere& sphere = *static_cast<const Sphere*>(&s);
		center = sphere.center;
		radius = sphere.radius;
	}
	return *this;
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
glm::vec3 Sphere::support(const glm::vec3& direction) const { return center + glm::normalize(direction) * radius; }
glm::mat3 Sphere::computeInertiaMatrix() const
{
	return glm::mat3(2.f / 5.f * radius * radius);
}
void Sphere::getFacingFace(const glm::vec3& direction, std::vector<glm::vec3>& points) const
{
	points.push_back(center + glm::normalize(direction) * radius);
}