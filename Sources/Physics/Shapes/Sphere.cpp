#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>


Sphere::Sphere(const glm::vec4& position, const float& r) : Shape(ShapeType::SPHERE), center(position), radius(r) 
{
	center.w = 1.f;
}
Sphere Sphere::toSphere() const { return *this; }
AxisAlignedBox Sphere::toAxisAlignedBox() const
{
	glm::vec4 r = glm::vec4(radius, radius, radius, 0);
	return AxisAlignedBox(center - r, center + r);
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
void Sphere::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), (glm::vec3)position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	center = m * center;
	radius = radius * glm::compMax(scale);
}
Shape* Sphere::duplicate() const { return new Sphere(*this); }
glm::vec4 Sphere::support(const glm::vec4& direction) const { return center + glm::normalize(direction) * radius; }
glm::mat3 Sphere::computeInertiaMatrix() const
{
	return glm::mat3(2.f / 5.f * radius * radius);
}
void Sphere::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	points.push_back(center + glm::normalize(direction) * radius);
}