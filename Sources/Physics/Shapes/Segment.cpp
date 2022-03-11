#include "Segment.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


Segment::Segment(const glm::vec4& a, const glm::vec4& b) : Shape(ShapeType::SEGMENT), p1(a), p2(b) {}
Sphere Segment::toSphere() const { return Sphere(0.5f*(p1 + p2), 0.5f*glm::length(p1 - p2)); }
AxisAlignedBox Segment::toAxisAlignedBox() const
{
	return AxisAlignedBox(glm::min(p1, p2), glm::max(p1, p2));
}
Shape& Segment::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::SEGMENT)
	{
		const Segment& segment = *static_cast<const Segment*>(&s);
		p1 = segment.p1;
		p2 = segment.p2;
	}
	return *this;
}
void Segment::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), (glm::vec3)position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p1 = m * p1;
	p2 = m * p2;
}
Shape* Segment::duplicate() const { return new Segment(*this); }
glm::vec4 Segment::support(const glm::vec4& direction) const
{
	if (glm::dot(p1, direction) > glm::dot(p2, direction))
		return p1;
	else return p2;
}
void Segment::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	points.push_back(p1);
	points.push_back(p2);
}