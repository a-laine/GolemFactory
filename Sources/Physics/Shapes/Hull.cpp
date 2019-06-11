#include "Hull.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"

#include "Resources/ResourceManager.h"
#include "Resources/Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

//	Default
//Hull::Hull(const std::vector<glm::vec3>& v, const std::vector<unsigned short>& f) : Shape(HULL), vertices(v), faces(f), degenerate(false){}
Hull::Hull(Mesh* m) : Shape(HULL), mesh(m)
{
	//initializePolyhedron(m);
	//quickhull(mesh);
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
	return AxisAlignedBox(min, max);
}
void Hull::operator=(const Shape& s)
{
	if (s.type == Shape::HULL)
	{
		const Hull& h = *static_cast<const Hull*>(&s);
		mesh = h.mesh;;
	}
}
void Hull::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation)
{
	glm::mat4 m = glm::translate(glm::mat4(1.0), position);
	m = m * glm::toMat4(orientation);
	m = glm::scale(m, scale);
	auto vertices = *mesh->getVertices();
	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i] = glm::vec3(m * glm::vec4(vertices[i], 1.f));
}
Shape* Hull::duplicate() const
{
	ResourceManager::getInstance()->getResource(mesh);
	return new Hull(mesh);
}
glm::vec3 Hull::GJKsupport(const glm::vec3& direction) const
{
	float d = std::numeric_limits<float>::min();
	glm::vec3 v = glm::vec3(0.f);
	auto vertices = *mesh->getVertices();
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

Mesh* Hull::fromMesh(const Mesh* m)
{
	std::cout << "Hull creation from mesh : " << m->name << std::endl;

	HullMesh hull = initializePolyhedron(m);
	Mesh* mesh = new Mesh("hull_" + m->name);
	if (hull.degenerated)
	{
		std::cout << "  degenerated mesh !" << std::endl;
		/*std::vector<glm::vec3> vertex;
			vertex.push_back(glm::vec3(0));
		std::vector<glm::vec3> normal;
			normal.push_back(glm::vec3(0));
		std::vector<glm::vec3> color;
			color.push_back(glm::vec3(0));*/
		mesh->initialize(std::vector<glm::vec3>(), std::vector<glm::vec3>(), std::vector<glm::vec3>(), hull.faces, std::vector<glm::ivec3>(), std::vector<glm::vec3>());
	}
	else
	{
		std::cout << "  good mesh" << std::endl;
		glm::vec3 hullColor = glm::vec3(0.5f, 0.f, 1.f);
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> colors;
		for (unsigned int i = 0; i < hull.normales.size(); i++)
		{
			normales.push_back(hull.normales[i]);
			normales.push_back(hull.normales[i]);
			normales.push_back(hull.normales[i]);

			colors.push_back(hullColor);
			colors.push_back(hullColor);
			colors.push_back(hullColor);
		}

		mesh->initialize(hull.vertices, normales, colors, hull.faces, std::vector<glm::ivec3>(), std::vector<glm::vec3>());
	}
	return mesh;
}
//

//	Private functions
Hull::HullMesh Hull::initializePolyhedron(const Mesh* mesh)
{
	HullMesh initialTetrahedron;

	// search maximum distance on axis.
	glm::vec3 min, max;
	std::vector<glm::vec3> axis;
		axis.push_back(glm::vec3(1, 0, 0));
		axis.push_back(glm::vec3(0, 1, 0));
		axis.push_back(glm::vec3(0, 0, 1));
	const std::vector<glm::vec3>& cloud = *mesh->getVertices();

	for (unsigned int i = 0; i < axis.size(); i++)
	{
		float mind = std::numeric_limits<float>::max();
		float maxd = std::numeric_limits<float>::min();

		for (unsigned int j = 0; j < cloud.size(); j++)
		{
			float d = glm::dot(axis[i], cloud[j]);
			if (d < mind)
			{
				mind = d;
				min = cloud[j];
			}
			if (d > maxd)
			{
				maxd = d;
				max = cloud[j];
			}
		}
		if (glm::any(glm::notEqual(min, max)))
			break;
	}
	if (glm::all(glm::equal(min, max))) // mesh is 1D or 2D
	{
		std::cout << "  error 1" << std::endl;
		initialTetrahedron.degenerated = true;
		return initialTetrahedron;
	}

	// search maximum distant point from initial segment
	float maxd = std::numeric_limits<float>::min();
	glm::vec3 T;
	glm::vec3 u = glm::normalize(max - min);
	for (unsigned int j = 0; j < cloud.size(); j++)
	{
		float d = glm::length(glm::cross(cloud[j] - min, u));
		if(d > maxd)
		{
			maxd = d;
			T = cloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 1D
	{
		std::cout << "  error 2" << std::endl;
		initialTetrahedron.degenerated = true;
		return initialTetrahedron;
	}

	// search maximum distant point from triangle
	maxd = std::numeric_limits<float>::min();
	glm::vec3 P;
	glm::vec3 n = glm::normalize(glm::cross(T - min, u));
	for (unsigned int j = 0; j < cloud.size(); j++)
	{
		float d = std::abs(glm::dot(cloud[j], n));
		if (d > maxd)
		{
			maxd = d;
			P = cloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 2D
	{
		std::cout << "  error 3" << std::endl;
		initialTetrahedron.degenerated = true;
		return initialTetrahedron;
	}

	// construct tetrahedron
	initialTetrahedron.degenerated = false;

	initialTetrahedron.vertices.push_back(min);
	initialTetrahedron.vertices.push_back(max);
	initialTetrahedron.vertices.push_back(T);
	initialTetrahedron.vertices.push_back(P);

	initialTetrahedron.faces.push_back(0);
	initialTetrahedron.faces.push_back(1);
	initialTetrahedron.faces.push_back(2);

	initialTetrahedron.faces.push_back(0);
	initialTetrahedron.faces.push_back(1);
	initialTetrahedron.faces.push_back(3);

	initialTetrahedron.faces.push_back(0);
	initialTetrahedron.faces.push_back(2);
	initialTetrahedron.faces.push_back(3);

	initialTetrahedron.faces.push_back(1);
	initialTetrahedron.faces.push_back(2);
	initialTetrahedron.faces.push_back(3);

	initialTetrahedron.normales.push_back(glm::normalize(glm::cross(max - min, T - min)));
	initialTetrahedron.normales.push_back(glm::normalize(glm::cross(max - min, P - min)));
	initialTetrahedron.normales.push_back(glm::normalize(glm::cross(T - min, P - min)));
	initialTetrahedron.normales.push_back(glm::normalize(glm::cross(T - max, P - max)));

	if (glm::dot(initialTetrahedron.normales[0], P) < 0)
		initialTetrahedron.normales[0] *= -1.f;
	if (glm::dot(initialTetrahedron.normales[1], T) < 0)
		initialTetrahedron.normales[1] *= -1.f;
	if (glm::dot(initialTetrahedron.normales[2], max) < 0)
		initialTetrahedron.normales[2] *= -1.f;
	if (glm::dot(initialTetrahedron.normales[3], min) < 0)
		initialTetrahedron.normales[3] *= -1.f;

	return initialTetrahedron;
}
void Hull::quickhull(const Mesh* mesh)
{

}
//