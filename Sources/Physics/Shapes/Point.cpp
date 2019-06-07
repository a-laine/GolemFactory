#include "Point.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Point::Point(const glm::vec3& position) : Shape(POINT), p(position) {}
Sphere Point::toSphere() const { return Sphere(p, 0.f); }
AxisAlignedBox Point::toAxisAlignedBox() const { return AxisAlignedBox(p, p); }
void Point::operator=(const Shape& s)
{
	if (s.type == Shape::POINT)
	{
		const Point& point = *static_cast<const Point*>(&s);
		p = point.p;
	}
};
void Point::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p = glm::vec3(m * glm::vec4(p, 1.f));
}
Shape* Point::duplicate() const { return new Point(*this); }
glm::vec3 Point::GJKsupport(const glm::vec3& direction) const { return p; }