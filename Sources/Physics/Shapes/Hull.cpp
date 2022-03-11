#include "Hull.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Utiles/ToolBox.h>

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

//	Default
Hull::Hull(Mesh* m, glm::mat4 transform) : Shape(ShapeType::HULL), base(transform), mesh(m)
{
	ResourceManager::getInstance()->getResource(m);
}
Hull::~Hull()
{
	ResourceManager::getInstance()->release(mesh);
}
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

	glm::vec4 p1 = base * glm::vec4(min, 1.f);
	glm::vec4 p2 = base * glm::vec4(max, 1.f);
	return AxisAlignedBox(glm::min(p1, p2), glm::max(p1, p2));
}
Shape& Hull::operator=(const Shape& s)
{
	if (s.type == Shape::ShapeType::HULL)
	{
		const Hull& h = *static_cast<const Hull*>(&s);
		mesh = h.mesh;
		base = h.base;
		ResourceManager::getInstance()->getResource(mesh);
	}
	return *this;
}
void Hull::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), (glm::vec3)position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	base = m * base;
}
Shape* Hull::duplicate() const
{
	ResourceManager::getInstance()->getResource(mesh);
	return new Hull(mesh, base);
}
glm::vec4 Hull::support(const glm::vec4& direction) const
{
	glm::vec4 u = glm::inverse(base) * direction;
	float d = std::numeric_limits<float>::min();
	glm::vec4 v = glm::vec4(0.f);
	const std::vector<glm::vec3>& vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		float a = glm::dot(vertices[i], (glm::vec3)u);
		if (a > d)
		{
			d = a;
			v = glm::vec4(vertices[i], 1);
		}
	}
	return base * v;
}
void Hull::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const
{
	glm::vec4 localDirection = glm::inverse(base) * direction;
	const std::vector<glm::vec3>& vertices = *mesh->getVertices();
	const std::vector<unsigned short>& faces = *mesh->getFaces();
	const std::vector<glm::vec3>& normals = *mesh->getNormals();
	float dotMax = std::numeric_limits<float>::min();
	
	unsigned int mostFace = 0;
	for (unsigned int i = 0; i < faces.size(); i += 3)
	{
		float dot = glm::dot(normals[i], (glm::vec3)localDirection);
		if (dot > dotMax)
		{
			dotMax = dot;
			mostFace = i;
		}
	}

	points.push_back(base * glm::vec4(vertices[mostFace], 1.f));
	points.push_back(base * glm::vec4(vertices[mostFace + 1], 1.f));
	points.push_back(base * glm::vec4(vertices[mostFace + 2], 1.f));
}
//