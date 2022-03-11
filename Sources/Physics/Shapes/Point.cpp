#include "Point.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Point::Point(const glm::vec4& position) : Shape(ShapeType::POINT), p(position) {}
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
void Point::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), (glm::vec3)position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p = m * p;
}
Shape* Point::duplicate() const { return new Point(*this); }
glm::vec4 Point::support(const glm::vec4& direction) const { return p; }
void Point::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	points.push_back(p);
}