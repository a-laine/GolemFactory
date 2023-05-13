#include "Point.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/

Point::Point(const vec4f& position) : Shape(ShapeType::POINT), p(position) {}
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
void Point::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	p = m * p;
}
Shape* Point::duplicate() const { return new Point(*this); }
vec4f Point::support(const vec4f& direction) const { return p; }
void Point::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	points.push_back(p);
}