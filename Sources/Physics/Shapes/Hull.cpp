#include "Hull.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include "Resources/ResourceManager.h"
#include "Resources/Mesh.h"
#include "Utiles/ToolBox.h"

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

//	Default
Hull::Hull(Mesh* m, glm::mat4 transform) : Shape(HULL), base(transform), mesh(m)
{}
//

//	Public functions
Sphere Hull::toSphere() const { return toAxisAlignedBox().toSphere(); }
AxisAlignedBox Hull::toAxisAlignedBox() const
{
	glm::vec3 max(std::numeric_limits<float>::min());
	glm::vec3 min(std::numeric_limits<float>::max());
	auto vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].x > max.x) max.x = vertices[i].x;
		if (vertices[i].y > max.y) max.y = vertices[i].y;
		if (vertices[i].z > max.z) max.z = vertices[i].z;

		if (vertices[i].x < min.x) min.x = vertices[i].x;
		if (vertices[i].y < min.y) min.y = vertices[i].y;
		if (vertices[i].z < min.z) min.z = vertices[i].z;
	}
	glm::vec3 p1 = glm::vec3(base*glm::vec4(min, 1.f));
	glm::vec3 p2 = glm::vec3(base*glm::vec4(max, 1.f));
	return AxisAlignedBox(glm::min(p1, p2), glm::max(p1, p2));
}
void Hull::operator=(const Shape& s)
{
	if (s.type == Shape::HULL)
	{
		const Hull& h = *static_cast<const Hull*>(&s);
		mesh = h.mesh;
		base = h.base;
	}
}
void Hull::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	base = m * base;
	//auto vertices = *mesh->getVertices();
	//for (unsigned int i = 0; i < vertices.size(); i++)
	//	vertices[i] = glm::vec3(m * glm::vec4(vertices[i], 1.f));
}
Shape* Hull::duplicate() const
{
	ResourceManager::getInstance()->getResource(mesh);
	return new Hull(mesh, base);
}
glm::vec3 Hull::GJKsupport(const glm::vec3& direction) const
{
	glm::vec3 u = glm::vec3(glm::inverse(base) * glm::vec4(direction, 1.f));
	float d = std::numeric_limits<float>::min();
	glm::vec3 v = glm::vec3(0.f);
	auto vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		float a = glm::dot(vertices[i], u);
		if (a > d)
		{
			d = a;
			v = vertices[i];
		}
	}
	return v;
}
//