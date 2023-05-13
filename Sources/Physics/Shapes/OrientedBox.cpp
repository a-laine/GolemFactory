#include "OrientedBox.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <vector>


OrientedBox::OrientedBox(const mat4f& transformationMatrix, const vec4f& localMin, const vec4f& localMax)
	: Shape(ShapeType::ORIENTED_BOX), base(transformationMatrix), min(localMin), max(localMax) {}
Sphere OrientedBox::toSphere() const
{
	vec4f p1 = base * min;
	vec4f p2 = base * max;
	return Sphere(0.5f * (p1 + p2), 0.5f * (p2 - p1).getNorm());
}
AxisAlignedBox OrientedBox::toAxisAlignedBox() const
{
	vec4f s = 0.5f * (max - min);
	vec4f sx = s.x * base[0];
	vec4f sy = s.y * base[1];
	vec4f sz = s.z * base[2];
	vec4f c[8];
	c[0] =  sx + sy + sz;
	c[1] =  sx + sy - sz;
	c[2] =  sx - sy + sz;
	c[3] =  sx - sy - sz;
	c[4] = -sx + sy + sz;
	c[5] = -sx + sy - sz;
	c[6] = -sx - sy + sz;
	c[7] = -sx - sy - sz;

	AxisAlignedBox box;
	box.min = c[0];
	box.max = c[0];
	for (int i = 1; i < 8; i++)
	{
		box.min = vec4f::min(box.min, c[i]);
		box.max = vec4f::max(box.max, c[i]);
	}

	vec4f center = base * (0.5f * (max + min));
	box.min += center;
	box.max += center;
	return box;
}
Shape& OrientedBox::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::ORIENTED_BOX)
	{
		const OrientedBox& box = *static_cast<const OrientedBox*>(&s);
		base = box.base;
		min = box.min;
		max = box.max;
	}
	return *this;
}
void OrientedBox::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	base = m * base;
	min = min;
	max = max;
}
Shape* OrientedBox::duplicate() const { return new OrientedBox(*this); }
vec4f OrientedBox::support(const vec4f& direction) const
{
	vec4f size = 0.5f * (max - min);
	vec4f x = base[0];
	vec4f y = base[1];
	vec4f z = base[2];

	vec4f result = base * (0.5f * (max + min));
	result += vec4f::dot(x, direction) > 0.f ? size.x * x : -(size.x) * x;
	result += vec4f::dot(y, direction) > 0.f ? size.y * y : -(size.y) * y;
	result += vec4f::dot(z, direction) > 0.f ? size.z * z : -(size.z) * z;

	return result;
}
void OrientedBox::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	vec4f center = base * (0.5f * (max + min));
	vec4f size = 0.5f * (max - min);
	vec4f x = base[0];  float dotx = vec4f::dot(x, direction);
	vec4f y = base[1];  float doty = vec4f::dot(y, direction);
	vec4f z = base[2];  float dotz = vec4f::dot(z, direction);
	vec4f u = vec4f::abs(vec4f(dotx, doty, dotz, 0));

	vec4f a2, a3;
	if (u.x > u.y && u.x > u.z)
	{
		center += dotx > 0.f ? size.x * x : -size.x * x;
		a2 = size.y * y;
		a3 = size.z * z;
	}
	else if (u.y > u.x && u.y > u.z)
	{
		center += doty > 0.f ? size.y * y : -size.y * y;
		a2 = size.x * x;
		a3 = size.z * z;
	}
	else
	{
		center += dotz > 0.f ? size.z * z : -size.z * z;
		a2 = size.y * y;
		a3 = size.x * x;
	}

	points.push_back(center + a2 + a3);
	points.push_back(center + a2 - a3);
	points.push_back(center - a2 - a3);
	points.push_back(center - a2 + a3);
}
mat4f OrientedBox::computeInertiaMatrix() const
{
	vec4f size = 0.5f * (max - min);
	mat4f M(0.f);
	M[0][0] = 1.f / 12.f * (size.y * size.y + size.z * size.z);
	M[1][1] = 1.f / 12.f * (size.x * size.x + size.z * size.z);
	M[2][2] = 1.f / 12.f * (size.x * size.x + size.y * size.y);
	M[3][3] = 1.f;
	return M;
}
