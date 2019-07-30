#include "AxisAlignedBox.h"
#include "Sphere.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "OrientedBox.h"


AxisAlignedBox::AxisAlignedBox(const glm::vec3& cornerMin, const glm::vec3& cornerMax)
	: Shape(AXIS_ALIGNED_BOX), min(cornerMin), max(cornerMax) {}
Sphere AxisAlignedBox::toSphere() const { return Sphere(0.5f*(min + max), 0.5f*glm::length(min - max)); }
Shape& AxisAlignedBox::operator=(const Shape& s)
{
	if (s.type == Shape::AXIS_ALIGNED_BOX)
	{
		const AxisAlignedBox& box = *static_cast<const AxisAlignedBox*>(&s);
		min = box.min;
		max = box.max;
	}
	return *this;
}
AxisAlignedBox AxisAlignedBox::toAxisAlignedBox() const { return *this; }
void AxisAlignedBox::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	OrientedBox base = OrientedBox(glm::mat4(1.f), min, max);
	base.transform(position, scale, orientation);
	AxisAlignedBox result = base.toAxisAlignedBox();
	min = result.min; 
	max = result.max;
}
Shape* AxisAlignedBox::duplicate() const { return new AxisAlignedBox(*this); }
glm::vec3 AxisAlignedBox::GJKsupport(const glm::vec3& direction) const
{
	glm::vec3 support(0.f);

	if (direction.x > 0) support.x = max.x;
	else if (direction.x < 0) support.x = min.x;
	if (direction.y > 0) support.y = max.y;
	else if (direction.y < 0) support.y = min.y;
	if (direction.z > 0) support.z = max.z;
	else if(direction.z < 0) support.z = min.z;

	return support;
}