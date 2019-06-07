#include "Hull.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

//	Default
Hull::Hull(const std::vector<glm::vec3>& v, const std::vector<unsigned short>& f) : Shape(HULL), vertices(v), faces(f) {}
//

//	Public functions
Sphere Hull::toSphere() const { return toAxisAlignedBox().toSphere(); }
AxisAlignedBox Hull::toAxisAlignedBox() const
{
	glm::vec3 max(std::numeric_limits<float>::min());
	glm::vec3 min(std::numeric_limits<float>::max());
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].x > max.x) max.x = vertices[i].x;
		if (vertices[i].y > max.y) max.y = vertices[i].y;
		if (vertices[i].z > max.z) max.z = vertices[i].z;

		if (vertices[i].x < min.x) min.x = vertices[i].x;
		if (vertices[i].y < min.y) min.y = vertices[i].y;
		if (vertices[i].z < min.z) min.z = vertices[i].z;
	}
	return AxisAlignedBox(min, max);
}
void Hull::operator=(const Shape& s)
{
	if (s.type == Shape::HULL)
	{
		const Hull& h = *static_cast<const Hull*>(&s);
		vertices = h.vertices;
		faces = h.faces;
	}
}
void Hull::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i] = glm::vec3(m * glm::vec4(vertices[i], 1.f));
}
Shape* Hull::duplicate() const
{
	return new Hull(vertices, faces);
}
glm::vec3 Hull::GJKsupport(const glm::vec3& direction) const
{
	float d = std::numeric_limits<float>::min();
	glm::vec3 v = glm::vec3(0.f);
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		float a = glm::dot(vertices[i], direction);
		if (a > d)
		{
			d = a;
			v = vertices[i];
		}
	}
	return v;
}
//