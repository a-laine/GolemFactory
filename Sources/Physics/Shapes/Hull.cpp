#include "Hull.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Utiles/ToolBox.h>

#include <utility>
/*#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>*/

//	Default
Hull::Hull(Mesh* m, mat4f transform) : Shape(ShapeType::HULL), base(transform), mesh(m)
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
	vec4f max(std::numeric_limits<float>::min());
	vec4f min(std::numeric_limits<float>::max());
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

	vec4f p1 = base * min;
	vec4f p2 = base * max;
	return AxisAlignedBox(vec4f::min(p1, p2), vec4f::max(p1, p2));
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
void Hull::transform(const vec4f& position, const vec4f& scale, const quatf& orientation)
{
	mat4f m = mat4f::TRS(position, orientation, scale);
	base = m * base;
}
Shape* Hull::duplicate() const
{
	ResourceManager::getInstance()->getResource(mesh);
	return new Hull(mesh, base);
}
vec4f Hull::support(const vec4f& direction) const
{
	vec4f u = mat4f::inverse(base) * direction;
	float d = std::numeric_limits<float>::min();
	vec4f v = vec4f::zero;
	const std::vector<vec4f>& vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		float a = vec4f::dot(vertices[i], u);
		if (a > d)
		{
			d = a;
			v = vertices[i];
		}
	}
	return base * v;
}
void Hull::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const
{
	vec4f localDirection = mat4f::inverse(base) * direction;
	const std::vector<vec4f>& vertices = *mesh->getVertices();
	const std::vector<unsigned short>& faces = *mesh->getFaces();
	const std::vector<vec4f>& normals = *mesh->getNormals();
	float dotMax = std::numeric_limits<float>::min();
	
	unsigned int mostFace = 0;
	for (unsigned int i = 0; i < faces.size(); i += 3)
	{
		float dot = vec4f::dot(normals[i], localDirection);
		if (dot > dotMax)
		{
			dotMax = dot;
			mostFace = i;
		}
	}

	points.push_back(base * vertices[mostFace]);
	points.push_back(base * vertices[mostFace + 1]);
	points.push_back(base * vertices[mostFace + 2]);
}
//