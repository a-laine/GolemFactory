#include "Segment.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/


Segment::Segment(const vec4f& a, const vec4f& b) : Shape(ShapeType::SEGMENT), p1(a), p2(b) {}
Sphere Segment::toSphere() const { return Sphere(0.5f * (p1 + p2), 0.5f * (p1 - p2).getNorm()); }
AxisAlignedBox Segment::toAxisAlignedBox() const
{
	return AxisAlignedBox(vec4f::min(p1, p2), vec4f::max(p1, p2));
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
void Segment::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	p1 = m * p1;
	p2 = m * p2;
}
Shape* Segment::duplicate() const { return new Segment(*this); }
vec4f Segment::support(const vec4f& direction) const
{
	if (vec4f::dot(p1, direction) > vec4f::dot(p2, direction))
		return p1;
	else return p2;
}
void Segment::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	points.push_back(p1);
	points.push_back(p2);
}