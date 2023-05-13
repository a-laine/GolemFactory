#include "Sphere.h"
#include "AxisAlignedBox.h"

/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/


Sphere::Sphere(const vec4f& position, const float& r) : Shape(ShapeType::SPHERE), center(position), radius(r) 
{
	center.w = 1.f;
}
Sphere Sphere::toSphere() const { return *this; }
AxisAlignedBox Sphere::toAxisAlignedBox() const
{
	vec4f r = vec4f(radius, radius, radius, 0);
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
void Sphere::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	center = m * center;
	float smax = std::max(scale.x, std::max(scale.y, scale.z));
	radius = radius * smax;
}
Shape* Sphere::duplicate() const { return new Sphere(*this); }
vec4f Sphere::support(const vec4f& direction) const { return center + direction.getNormal() * radius; }
mat4f Sphere::computeInertiaMatrix() const
{
	return mat4f(2.f / 5.f * radius * radius);
}
void Sphere::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	points.push_back(center + direction.getNormal() * radius);
}