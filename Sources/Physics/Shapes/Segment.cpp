#include "Segment.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


Segment::Segment(const glm::vec3& a, const glm::vec3& b) : Shape(SEGMENT), p1(a), p2(b) {}
Sphere Segment::toSphere() const { return Sphere(0.5f*(p1 + p2), 0.5f*glm::length(p1 - p2)); }
AxisAlignedBox Segment::toAxisAlignedBox() const
{
	return AxisAlignedBox(glm::min(p1, p2), glm::max(p1, p2));
}
void Segment::operator=(const Shape& s)
{
	if (s.type == Shape::SEGMENT)
	{
		const Segment& segment = *static_cast<const Segment*>(&s);
		p1 = segment.p1;
		p2 = segment.p2;
	}
}
void Segment::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	p1 = glm::vec3(m * glm::vec4(p1, 1.f));
	p2 = glm::vec3(m * glm::vec4(p2, 1.f));
}
Shape* Segment::duplicate() const { return new Segment(*this); }
glm::vec3 Segment::GJKsupport(const glm::vec3& direction) const
{
	if (glm::dot(p1, direction) > glm::dot(p2, direction))
		return p1;
	else return p2;
}