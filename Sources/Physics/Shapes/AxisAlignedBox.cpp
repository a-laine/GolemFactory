#include "AxisAlignedBox.h"
#include "Sphere.h"
#include "OrientedBox.h"

/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/



AxisAlignedBox::AxisAlignedBox(const vec4f& cornerMin, const vec4f& cornerMax)
	: Shape(ShapeType::AXIS_ALIGNED_BOX), min(cornerMin), max(cornerMax) 
{
	min.w = 1.f;
	max.w = 1.f;
}
Sphere AxisAlignedBox::toSphere() const { return Sphere(0.5f * (min + max), 0.5f * (min - max).getNorm()); }
Shape& AxisAlignedBox::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::AXIS_ALIGNED_BOX)
	{
		const AxisAlignedBox& box = *static_cast<const AxisAlignedBox*>(&s);
		min = box.min;
		max = box.max;
	}
	return *this;
}
AxisAlignedBox AxisAlignedBox::toAxisAlignedBox() const { return *this; }
void AxisAlignedBox::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	OrientedBox base = OrientedBox(mat4f::identity, min, max);
	base.transform(position, scale, orientation);
	AxisAlignedBox result = base.toAxisAlignedBox();
	min = result.min; 
	max = result.max;
}
Shape* AxisAlignedBox::duplicate() const { return new AxisAlignedBox(*this); }
vec4f AxisAlignedBox::support(const vec4f& direction) const
{
	vec4f support(0.f);

	if (direction.x >= 0.f) support.x = max.x;
	else support.x = min.x;
	if (direction.y >= 0.f) support.y = max.y;
	else support.y = min.y;
	if (direction.z >= 0.f) support.z = max.z;
	else support.z = min.z;

	return support;
}
void AxisAlignedBox::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	vec4f d = vec4f::abs(direction);

	if (d.x > d.y && d.x > d.z)
	{
		if (direction.x < 0.f)
		{
			points.push_back(min);
			points.push_back(vec4f(min.x, min.y, max.z, 1));
			points.push_back(vec4f(min.x, max.y, max.z, 1));
			points.push_back(vec4f(min.x, max.y, min.z, 1));
		}
		else
		{
			points.push_back(vec4f(max.x, min.y, min.z, 1));
			points.push_back(vec4f(max.x, min.y, max.z, 1));
			points.push_back(max);
			points.push_back(vec4f(max.x, max.y, min.z, 1));
		}
	}
	else if (d.y > d.x && d.y > d.z)
	{
		if (direction.y < 0.f)
		{
			points.push_back(min);
			points.push_back(vec4f(min.x, min.y, max.z, 1));
			points.push_back(vec4f(max.x, min.y, max.z, 1));
			points.push_back(vec4f(max.x, min.y, min.z, 1));
		}
		else
		{
			points.push_back(vec4f(min.x, max.y, min.z, 1));
			points.push_back(vec4f(min.x, max.y, max.z, 1));
			points.push_back(max);
			points.push_back(vec4f(max.x, max.y, min.z, 1));
		}
	}
	else
	{
		if (direction.z < 0.f)
		{
			points.push_back(min);
			points.push_back(vec4f(min.x, max.y, min.z, 1));
			points.push_back(vec4f(max.x, max.y, min.z, 1));
			points.push_back(vec4f(max.x, min.y, min.z, 1));
		}
		else
		{
			points.push_back(vec4f(min.x, min.y, max.z, 1));
			points.push_back(vec4f(min.x, max.y, max.z, 1));
			points.push_back(max);
			points.push_back(vec4f(max.x, min.y, max.z, 1));
		}
	}
}
mat4f AxisAlignedBox::computeInertiaMatrix() const
{
	vec4f size = 0.5f * (max - min);
	mat4f M(0.f);
	M[0][0] = 1.f / 12.f * (size.y * size.y + size.z * size.z);
	M[1][1] = 1.f / 12.f * (size.x * size.x + size.z * size.z);
	M[2][2] = 1.f / 12.f * (size.x * size.x + size.y * size.y);
	return M;
}


void AxisAlignedBox::add(const AxisAlignedBox& _other)
{
	min = vec4f::min(min, _other.min);
	max = vec4f::max(max, _other.max);
}
