#include "OrientedBox.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"
#include <Physics/SpecificCollision/CollisionUtils.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>


OrientedBox::OrientedBox(const glm::mat4& transformationMatrix, const glm::vec3& localMin, const glm::vec3& localMax)
	: Shape(ORIENTED_BOX), base(transformationMatrix), min(localMin), max(localMax) {}
Sphere OrientedBox::toSphere() const
{
	glm::vec3 p1 = glm::vec3(base*glm::vec4(min, 1.f));
	glm::vec3 p2 = glm::vec3(base*glm::vec4(max, 1.f));
	return Sphere(0.5f*(p1 + p2), 0.5f*glm::length(p2 - p1));
}
glm::mat4 absMat4(const glm::mat4& m)
{
	glm::mat4 r;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			r[i][j] = glm::abs(m[i][j]);
	return r;
}
AxisAlignedBox OrientedBox::toAxisAlignedBox() const
{
	glm::vec3 center = 0.5f * (max + min);
	glm::vec3 size = 0.5f * (max - min);

	glm::vec3 newCenter = glm::vec3((glm::translate(center) * base)[3]);
	glm::vec3 newSize =  glm::vec3(absMat4(base) * glm::vec4(size, 0.f));

	return AxisAlignedBox(newCenter - newSize, newCenter + newSize);
}
Shape& OrientedBox::operator=(const Shape& s)
{
	if (s.type == Shape::ORIENTED_BOX)
	{
		const OrientedBox& box = *static_cast<const OrientedBox*>(&s);
		base = box.base;
		min = box.min;
		max = box.max;
	}
	return *this;
}
void OrientedBox::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), position);
	m = m * glm::toMat4(orientation);

	base = m * base;

	min = min * scale;
	max = max * scale;
}
Shape* OrientedBox::duplicate() const { return new OrientedBox(*this); }
glm::vec3 OrientedBox::GJKsupport(const glm::vec3& direction) const
{
	glm::vec3 d = glm::vec3(glm::inverse(base) * glm::vec4(direction, 0.f));
	glm::vec3 support = glm::vec3(0.f);

	if (d.x > COLLISION_EPSILON) support.x = max.x;
	else if(d.x < -COLLISION_EPSILON) support.x = min.x;
	if (d.y > COLLISION_EPSILON) support.y = max.y;
	else if (d.y < -COLLISION_EPSILON) support.y = min.y;
	if (d.z > COLLISION_EPSILON) support.z = max.z;
	else if (d.z < -COLLISION_EPSILON) support.z = min.z;

	return glm::vec3(base * glm::vec4(support, 1.f));
}
