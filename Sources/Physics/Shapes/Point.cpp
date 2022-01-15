#include "Point.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Point::Point(const glm::vec3& position) : Shape(ShapeType::POINT), p(position) {}
Sphere Point::toSphere() const { return Sphere(p, 0.f); }
AxisAlignedBox Point::toAxisAlignedBox() const { return AxisAlignedBox(p, p); }
Shape& Point::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::POINT)
	{
		const Point& point = *static_cast<const Point*>(&s);
		p = point.p;
	}
	return *this;
};
void Point::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p = glm::vec3(m * glm::vec4(p, 1.f));
}
Shape* Point::duplicate() const { return new Point(*this); }
glm::vec3 Point::support(const glm::vec3& direction) const { return p; }
void Point::getFacingFace(const glm::vec3& direction, std::vector<glm::vec3>& points) const
{
	points.push_back(p);
}