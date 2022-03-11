#include "OrientedBox.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"
//#include <Physics/SpecificCollision/CollisionUtils.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>


OrientedBox::OrientedBox(const glm::mat4& transformationMatrix, const glm::vec4& localMin, const glm::vec4& localMax)
	: Shape(ShapeType::ORIENTED_BOX), base(transformationMatrix), min(localMin), max(localMax) {}
Sphere OrientedBox::toSphere() const
{
	glm::vec4 p1 = base * min;
	glm::vec4 p2 = base * max;
	return Sphere(0.5f * (p1 + p2), 0.5f * glm::length(p2 - p1));
}
AxisAlignedBox OrientedBox::toAxisAlignedBox() const
{
	auto absMat4 = [](const glm::mat4 & _m) // make abs for each element of the rotation part
	{
		glm::mat4 r;
		for (int i = 0; i < 3; i++)
			r[i] = glm::abs(_m[i]);
		return r;
	};

	glm::vec4 center = base * (0.5f * (max + min));
	glm::vec4 size =  absMat4(base) * (0.5f * (max - min));
	return AxisAlignedBox(center - size, center + size);
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
void OrientedBox::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), (glm::vec3)position);
	m = m * glm::toMat4(orientation);

	base = m * base;

	min = min * glm::vec4(scale, 1.f);
	max = max * glm::vec4(scale, 1.f);
}
Shape* OrientedBox::duplicate() const { return new OrientedBox(*this); }
glm::vec4 OrientedBox::support(const glm::vec4& direction) const
{
	glm::vec4 size = 0.5f * (max - min);
	glm::vec4 x = base[0];
	glm::vec4 y = base[1];
	glm::vec4 z = base[2];

	glm::vec4 result = base * (0.5f * (max + min));
	result += glm::dot(x, direction) > 0.f ? size.x * x : -(size.x) * x;
	result += glm::dot(y, direction) > 0.f ? size.y * y : -(size.y) * y;
	result += glm::dot(z, direction) > 0.f ? size.z * z : -(size.z) * z;

	return result;
}
void OrientedBox::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	glm::vec4 center = base * (0.5f * (max + min));
	glm::vec4 size = 0.5f * (max - min);
	glm::vec4 x = base[0];  float dotx = glm::dot(x, direction);
	glm::vec4 y = base[1];  float doty = glm::dot(y, direction);
	glm::vec4 z = base[2];  float dotz = glm::dot(z, direction);
	glm::vec4 u = glm::abs(glm::vec4(dotx, doty, dotz, 0));

	glm::vec4 a2, a3;
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
glm::mat3 OrientedBox::computeInertiaMatrix() const
{
	glm::vec4 size = 0.5f * (max - min);
	glm::mat3 M(0.f);
	M[0][0] = 1.f / 12.f * (size.y * size.y + size.z * size.z);
	M[1][1] = 1.f / 12.f * (size.x * size.x + size.z * size.z);
	M[2][2] = 1.f / 12.f * (size.x * size.x + size.y * size.y);
	return M;
}
