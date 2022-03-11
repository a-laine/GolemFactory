#include "AxisAlignedBox.h"
#include "Sphere.h"
#include "OrientedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>



AxisAlignedBox::AxisAlignedBox(const glm::vec4& cornerMin, const glm::vec4& cornerMax)
	: Shape(ShapeType::AXIS_ALIGNED_BOX), min(cornerMin), max(cornerMax) 
{
	min.w = 1.f;
	max.w = 1.f;
}
Sphere AxisAlignedBox::toSphere() const { return Sphere(0.5f*(min + max), 0.5f*glm::length(min - max)); }
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
void AxisAlignedBox::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	OrientedBox base = OrientedBox(glm::mat4(1.f), min, max);
	base.transform(position, scale, orientation);
	AxisAlignedBox result = base.toAxisAlignedBox();
	min = result.min; 
	max = result.max;
}
Shape* AxisAlignedBox::duplicate() const { return new AxisAlignedBox(*this); }
glm::vec4 AxisAlignedBox::support(const glm::vec4& direction) const
{
	glm::vec4 support(0.f);

	if (direction.x >= 0.f) support.x = max.x;
	else support.x = min.x;
	if (direction.y >= 0.f) support.y = max.y;
	else support.y = min.y;
	if (direction.z >= 0.f) support.z = max.z;
	else support.z = min.z;

	return support;
}
void AxisAlignedBox::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	glm::vec4 d = glm::abs(direction);

	if (d.x > d.y && d.x > d.z)
	{
		if (direction.x < 0.f)
		{
			points.push_back(min);
			//points.push_back(glm::vec3(min.x, min.y, min.z));
			points.push_back(glm::vec4(min.x, min.y, max.z, 1));
			points.push_back(glm::vec4(min.x, max.y, max.z, 1));
			points.push_back(glm::vec4(min.x, max.y, min.z, 1));
		}
		else
		{
			points.push_back(glm::vec4(max.x, min.y, min.z, 1));
			points.push_back(glm::vec4(max.x, min.y, max.z, 1));
			//points.push_back(glm::vec3(max.x, max.y, max.z));
			points.push_back(max);
			points.push_back(glm::vec4(max.x, max.y, min.z, 1));
		}
	}
	else if (d.y > d.x && d.y > d.z)
	{
		if (direction.y < 0.f)
		{
			points.push_back(min);
			//points.push_back(glm::vec3(min.x, min.y, min.z));
			points.push_back(glm::vec4(min.x, min.y, max.z, 1));
			points.push_back(glm::vec4(max.x, min.y, max.z, 1));
			points.push_back(glm::vec4(max.x, min.y, min.z, 1));
		}
		else
		{
			points.push_back(glm::vec4(min.x, max.y, min.z, 1));
			points.push_back(glm::vec4(min.x, max.y, max.z, 1));
			//points.push_back(glm::vec3(max.x, max.y, max.z));
			points.push_back(max);
			points.push_back(glm::vec4(max.x, max.y, min.z, 1));
		}
	}
	else
	{
		if (direction.z < 0.f)
		{
			points.push_back(min);
			//points.push_back(glm::vec3(min.x, min.y, min.z));
			points.push_back(glm::vec4(min.x, max.y, min.z, 1));
			points.push_back(glm::vec4(max.x, max.y, min.z, 1));
			points.push_back(glm::vec4(max.x, min.y, min.z, 1));
		}
		else
		{
			points.push_back(glm::vec4(min.x, min.y, max.z, 1));
			points.push_back(glm::vec4(min.x, max.y, max.z, 1));
			//points.push_back(glm::vec3(max.x, max.y, max.z));
			points.push_back(max);
			points.push_back(glm::vec4(max.x, min.y, max.z, 1));
		}
	}
}
glm::mat3 AxisAlignedBox::computeInertiaMatrix() const
{
	glm::vec4 size = 0.5f * (max - min);
	glm::mat3 M(0.f);
	M[0][0] = 1.f / 12.f * (size.y * size.y + size.z * size.z);
	M[1][1] = 1.f / 12.f * (size.x * size.x + size.z * size.z);
	M[2][2] = 1.f / 12.f * (size.x * size.x + size.y * size.y);
	return M;
}


void AxisAlignedBox::add(const AxisAlignedBox& _other)
{
	min = glm::min(min, _other.min);
	max = glm::max(max, _other.max);
}
